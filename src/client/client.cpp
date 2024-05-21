#include "client.h"

#include "version_info.h"
#include "utils.h"

#include <asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <functional>
#include <fstream>

Client::Client(asio::io_context &io_context, const std::string server_ip,
               const uint16_t port, const double range_constant)
    : io_context_(io_context),
      network_(io_context, GetCallbackList()),
      range_constant_(range_constant), first_delay_timer_(io_context),
      repeat_(io_context)
{
    udp::resolver resolver(io_context);

    server_endpoint_ =
        *resolver.resolve(udp::v4(), server_ip, std::to_string(port)).begin();

    repeat_.SetCallback([this]()
                        {
            spdlog::error("Unable to connect to {}:{} after {} attempts. Exiting.",
            server_endpoint_.address().to_string(), server_endpoint_.port(), constants::send_attempts);
            io_context_.stop();
            exit; });
}

void Client::start()
{
    MakeInitialRequest();
    network_.Receive();
}

ReceiveHandlingFuncs Client::GetCallbackList()
{
    ReceiveHandlingFuncs callbacks;

    callbacks.insert({PacketType::ServerResponse,
                      [this](udp::endpoint sender)
                      {
                          ServerResponse::Ptr packet = std::make_shared<ServerResponse>();
                          network_.GetPacket(packet);

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandleSeverResponse,
                                               this, packet, sender));
                      }});

    callbacks.insert({PacketType::PacketCheckRequest,
                      [this](udp::endpoint sender)
                      {
                          PacketCheckRequest::Ptr packet = std::make_shared<PacketCheckRequest>();
                          network_.GetPacket(packet);

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandlePacketCheckRequest,
                                               this, packet, sender));
                      }});

    callbacks.insert({PacketType::PayloadMessage,
                      [this](udp::endpoint sender)
                      {
                          PayloadMessage::Ptr packet = std::make_shared<PayloadMessage>();
                          if (packet.get() == nullptr)
                              spdlog::critical("Not allocated!!!");
                          network_.GetPacket(packet);

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandlePayloadMessage,
                                               this, packet, sender));
                      }});

    return callbacks;
}

void Client::MakeInitialRequest()
{
    InitialRequest::Ptr request = std::make_shared<InitialRequest>();

    request->major_version_ = project::major_version;
    request->minor_version_ = project::minor_version;
    request->patch_version_ = project::patch_version;

    spdlog::info("Sending version info to server. Version: {}", project::version_string);

    repeat_(std::bind(&Network::Send, &network_, request, server_endpoint_));
}

void Client::SendRangeSetting()
{
    spdlog::info("Sending constant: {}.", range_constant_);

    RangeSettingMessage::Ptr setting = std::make_shared<RangeSettingMessage>();
    setting->range_constant_ = range_constant_;

    network_.Send(setting, server_endpoint_);
}

void Client::FlushBuffer()
{
    for (int i = 0; i < buffer_.size(); ++i)
    {
        for (int j = 0; j < buffer_[i]->payload_size_; ++j)
            collected_data_.push_back(buffer_[i]->payload_[j]);
    }
    spdlog::trace("Flushing {} packets from buffer.", buffer_.size());
    buffer_.clear();
}

void Client::HandleSeverResponse(const ServerResponse::Ptr response, const udp::endpoint sender)
{
    repeat_.stop();

    // TODO Separate initialization and error handling logic
    if (!response->is_successful_)
    {
        spdlog::error("Rejected by server: {}\nStopping.", response->message_);
        io_context_.stop();
        return;
    }

    spdlog::info("Request was accepted by server.");

    first_delay_timer_.expires_from_now(asio::chrono::seconds(3));
    first_delay_timer_.async_wait(std::bind(&Client::SendRangeSetting, this));
}

void Client::HandlePayloadMessage(const PayloadMessage::Ptr packet, const udp::endpoint sender)
{
    Lock lock(data_mutex_);

    if (packet.get() == nullptr)
    {
        spdlog::critical("Pointer is null!");
    }

    if (packet->packet_id_ == 0)
    {
        spdlog::error("Ignorring packet with null id.");
        return;
    }

    spdlog::trace("Received payload packet: {} Size: {}", packet->packet_id_, packet->payload_size_);
    buffer_.push_back(packet);
}

void Client::HandlePacketCheckRequest(const PacketCheckRequest::Ptr request, const udp::endpoint sender)
{

    PacketCheckResponse::Ptr response = std::make_shared<PacketCheckResponse>();

    {
        Lock lock(data_mutex_);

        if (request->packets_sent_ == 0)
            return ProcessData();

        spdlog::info("Incoming packet check request. packets sent: {}", request->packets_sent_);

        std::sort(buffer_.begin(), buffer_.end(), [](const auto &lhs, const auto &rhs)
                  { return lhs->packet_id_ < rhs->packet_id_; });

        std::unique(buffer_.begin(), buffer_.end(), [](const auto &lhs, const auto &rhs)
                    { return lhs->packet_id_ == rhs->packet_id_; });

        response->packets_missing_ = 0;

        size_t pos = 0;
        for (size_t i = 1; i <= request->packets_sent_; ++i)
        {
            if (pos >= buffer_.size() || buffer_[pos]->packet_id_ != i)
            {
                response->missing_packets_[response->packets_missing_] = i;
                ++response->packets_missing_;

                spdlog::trace("Adding to request missing packet: {}", i);
            }
            else
                ++pos;
        }

        if (response->packets_missing_ == 0)
            FlushBuffer();
    }

    spdlog::warn("Requesting {} missing packets from server.", response->packets_missing_);
    network_.Send(response, sender);
}

void Client::ProcessData()
{
    FlushBuffer();
    std::sort(collected_data_.begin(), collected_data_.end(), std::greater());

    spdlog::info("Writing received data to file");

    auto path = utils::CurrentExecutableFilePath().replace_filename("data.bin");
    std::ofstream binary_file(path);
    std::ostream_iterator<double> out(binary_file);

    std::copy(collected_data_.begin(), collected_data_.end(), out);

    binary_file.close();
}
