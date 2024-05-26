#include "client.h"

#include "version_info.h"
#include "utils.h"

#include <asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <functional>
#include <fstream>

using asio::chrono::milliseconds;

Client::Client(asio::io_context &io_context, const std::string server_ip,
               const uint16_t port, const double range_constant)
    : io_context_(io_context),
      network_(io_context, GetCallbackList(), server_ip, port),
      range_constant_(range_constant), timer_(io_context),
      is_waiting_for_resopnse(false), current_chunk_(1)
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
                          ServerResponse::Ptr packet = network_.GetReceivedPacket<ServerResponse>();

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandleSeverResponse,
                                               this, packet, sender));
                      }});

    callbacks.insert({PacketType::PacketCheckRequest,
                      [this](udp::endpoint sender)
                      {
                          PacketCheckRequest::Ptr packet = network_.GetReceivedPacket<PacketCheckRequest>();

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandlePacketCheckRequest,
                                               this, packet, sender));
                      }});

    callbacks.insert({PacketType::PayloadMessage,
                      [this](udp::endpoint sender)
                      {
                          PayloadMessage::Ptr packet = network_.GetReceivedPacket<PayloadMessage>();

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
    request->chunk_ = 1;

    spdlog::info("Sending version info to server. Version: {}", project::version_string);
    is_waiting_for_resopnse = true;
    // RepeatedlySend(request, asio::chrono::milliseconds(500), asio::chrono::milliseconds(5000));
    network_.SerializeAndSend(request, server_endpoint_);
}

void Client::SendRangeSetting()
{
    spdlog::info("Sending constant: {}.", range_constant_);

    RangeSettingMessage::Ptr setting = std::make_shared<RangeSettingMessage>();
    setting->range_constant_ = range_constant_;
    setting->chunk_ = 1;

    is_waiting_for_resopnse = true;
    network_.SerializeAndSend(setting, server_endpoint_);
    // RepeatedlySend(setting, asio::chrono::milliseconds(10'000), asio::chrono::milliseconds(20'000));
}

void Client::FlushBuffer()
{
    for (const auto &packet : buffer_)
        for (int i = 0; i < packet->payload_size_; ++i)
            collected_data_.push_back(packet->payload_[i]);

    ++current_chunk_;
    spdlog::trace("Flushing {} packets from buffer.", buffer_.size());
    buffer_.clear();
}

// void Client::RepeatedlySend(const CommandPacket::Ptr packet, asio::chrono::milliseconds delay,
//                             asio::chrono::milliseconds max_delay)
// {
//     std::scoped_lock lock(data_mutex_);

//     if (!is_waiting_for_resopnse ||
//         packet->chunk_ < current_chunk_)
//         return;

//     if (delay > max_delay)
//     {
//         spdlog::error("Timeout exceeded. No response from server.");
//         io_context_.stop();
//         exit(1);
//     }

//     network_.SerializeAndSend(packet, server_endpoint_);

//     timer_.expires_from_now(delay);
//     timer_.async_wait(std::bind(&Client::RepeatedlySend,
//                                 this, packet, delay * 2, max_delay));
// }

void Client::HandleSeverResponse(const ServerResponse::Ptr response, const udp::endpoint sender)
{
    is_waiting_for_resopnse = false;

    if (response->chunk_ < current_chunk_)
        return;

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
    is_waiting_for_resopnse = false;

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

    if (request->chunk_ < current_chunk_)
    {
        is_waiting_for_resopnse = true;
        response->chunk_ = request->chunk_;
        response->packets_missing_ = 0;
        // RepeatedlySend(response, milliseconds(100), milliseconds(5000));
        network_.SerializeAndSend(response, server_endpoint_);

        return;
    }

    {
        Lock lock(data_mutex_);
        
        if (request->packets_sent_ == 0)
            return ProcessData();

        spdlog::info("Incoming packet check request to check {} packets from chunk: {}.", request->packets_sent_, request->chunk_);

        if (!buffer_.back())
            buffer_.pop_back();

        std::sort(buffer_.begin(), buffer_.end(), [](const auto &lhs, const auto &rhs)
                  { return lhs->packet_id_ < rhs->packet_id_; });

        auto new_end = std::unique(buffer_.begin(), buffer_.end(), [](const auto &lhs, const auto &rhs)
                                   { return lhs->packet_id_ == rhs->packet_id_; });

        buffer_.erase(new_end, buffer_.end());

        response->packets_missing_ = 0;
        response->chunk_ = current_chunk_;

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

        spdlog::warn("Requesting {} missing packets from server for chunk {}.", response->packets_missing_,
                     request->chunk_);

        is_waiting_for_resopnse = true;
    }
    network_.SerializeAndSend(response, server_endpoint_);
    // RepeatedlySend(response, milliseconds(100), milliseconds(5000));
}

void Client::ProcessData()
{
    FlushBuffer();
    std::sort(collected_data_.begin(), collected_data_.end(), std::greater());

    auto path = utils::CurrentExecutableFilePath().replace_filename("data.bin");
    std::ofstream binary_file(path, std::ofstream::binary);

    if (!binary_file)
        throw std::runtime_error("Can't open file for writing");

    const size_t data_size = collected_data_.size() * sizeof(collected_data_.front());
    spdlog::info("Writing received {} bytes to file", data_size);
    binary_file.write(reinterpret_cast<char *>(collected_data_.data()), data_size);

    binary_file.close();
    io_context_.stop();

    exit(0);
}
