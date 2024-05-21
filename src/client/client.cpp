#include "client.h"

#include "version_info.h"
#include "utils.h"

#include <asio/io_context.hpp>
// #include <asio/stream_file.hpp>
#include <spdlog/spdlog.h>

#include <functional>

Client::Client(asio::io_context &io_context, const std::string server_ip,
               const uint16_t port, const double range_constant)
    : io_context_(io_context),
      network_(io_context, GetCallbackList()),
      range_constant_(range_constant), timer_(io_context)
{
    udp::resolver resolver(io_context);

    server_endpoint_ =
        *resolver.resolve(udp::v4(), server_ip, std::to_string(port)).begin();
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

    network_.Send(request, server_endpoint_);
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
    for (const auto packet : buffer_)
        for (int i = 0; i < packet->payload_size_; ++i)
            collected_data_.push_back(packet->payload_[i]);

    spdlog::trace("Flushing {} packets from buffer.", buffer_.size());
    buffer_.clear();
}

void Client::HandleSeverResponse(const ServerResponse::Ptr response, const udp::endpoint sender)
{
    // TODO Separate initialization and error handling logic
    if (!response->is_successful_)
    {
        spdlog::error("Rejected by server: {}\nStopping.", response->message_);
        io_context_.stop();
        return;
    }

    spdlog::info("Request was accepted by server.");

    timer_.expires_from_now(asio::chrono::seconds(3));
    timer_.async_wait(std::bind(&Client::SendRangeSetting, this));
}

void Client::HandlePayloadMessage(const PayloadMessage::Ptr packet, const udp::endpoint sender)
{
    Lock lock(data_mutex_);

    if (packet->packet_id_ == 0)
    {
        spdlog::warn("Ignorring packet with null id.");
        return;
    }

    spdlog::trace("Received payload packet: {}", packet->packet_id_);
    buffer_.push_back(packet);
}

void Client::HandlePacketCheckRequest(const PacketCheckRequest::Ptr request, const udp::endpoint sender)
{
    Lock lock(data_mutex_);

    if (request->packets_sent_ == 0)
        return ProcessData();

    std::sort(buffer_.begin(), buffer_.end());
    std::unique(buffer_.begin(), buffer_.end());

    PacketCheckResponse::Ptr response = std::make_shared<PacketCheckResponse>();
    response->packets_missing_ = 0;

    if (buffer_.size() == request->packets_sent_)
    {
        spdlog::info("All {} packets from server were received successfully", request->packets_sent_);
        FlushBuffer();
        network_.Send(response, sender);
        return;
    }

    size_t pos = 0;
    for (size_t i = 1; i <= request->packets_sent_; ++i)
    {
        if (buffer_[pos]->packet_id_ != i)
        {
            response->missing_packets_[response->packets_missing_] = i;
            ++response->packets_missing_;
        }
        else
            ++pos;
    }

    spdlog::info("Requesting {} missing packets from server.", response->packets_missing_);
    network_.Send(response, sender);
}

void Client::ProcessData()
{
    Lock lock(data_mutex_);
    FlushBuffer();
    std::sort(collected_data_.begin(), collected_data_.end(), std::greater());

    // asio::stream_file file(
    //     io_context_, "/path/to/file",
    //     asio::stream_file::write_only | asio::stream_file::create);

    // file.async_write_some(collected_data_,
    //                       [this](std::error_code error, size_t bytes_written)
    //                       {
    //                           if (error)
    //                               throw std::runtime_error(error.message());
    //                           else
    //                               spdlog::info("{} bytes was written to result file");
    //                           io_context_.stop();
    //                       });
}
