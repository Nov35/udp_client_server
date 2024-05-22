#include "network.h"

#include "serializer.h"

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include <vector>

using asio::ip::udp;

Network::Network(asio::io_context& io_context, ReceiveHandlingFuncs&& packet_handle_callbacks, const std::string server_ip, const uint16_t port)
    : socket_(io_context), receive_buffer_(constants::max_packet_size),
      packet_handle_callbacks_(packet_handle_callbacks)
{

    udp::resolver resolver(io_context);

    auto&& server_endpoint_ =
        *resolver.resolve(udp::v4(), server_ip, std::to_string(port)).begin();

    asio::connect(socket_, resolver.resolve(udp::v4(), server_ip, std::to_string(port)));
}

Network::Network(asio::io_context &io_context, const uint16_t port,
                 ReceiveHandlingFuncs &&packet_handle_callbacks)
    : socket_(io_context, udp::endpoint(udp::v4(), port)),
      receive_buffer_(constants::max_packet_size),
      packet_handle_callbacks_(packet_handle_callbacks)
{
}

void Network::Send(const Packet::Ptr packet, const udp::endpoint receiver_endpoint)
{
    BinaryData buffer(constants::max_packet_size);
    size_t data_size = Serialize(packet, buffer);

    socket_.async_send_to(asio::buffer(buffer.data(), data_size), receiver_endpoint,
                          std::bind(&Network::HandleSend,
                                    this,
                                    asio::placeholders::error,
                                    asio::placeholders::bytes_transferred,
                                    receiver_endpoint));
}

void Network::Receive()
{
    socket_.async_receive_from(asio::buffer(receive_buffer_), last_sender_,
                               std::bind(&Network::HandleReceive,
                                         this,
                                         asio::placeholders::error,
                                         asio::placeholders::bytes_transferred));
}

void Network::GetPacket(Packet::Ptr destination)
{
    Deserialize(receive_buffer_, destination);
}

void Network::HandleSend(const std::error_code &error, const size_t bytes_transferred,
                         const udp::endpoint receiver_endpoint)
{
    if (error)
    {
        spdlog::error("{}:{}: Error during sending: {}",
                      receiver_endpoint.address().to_string(), receiver_endpoint.port(),
                      error.message());
        return;
    }
}

void Network::HandleReceive(const std::error_code &error,
                            const size_t bytes_received)
{
    if (error)
    {
        spdlog::error("{}:{}: Error during receiving: {}",
            last_sender_.address().to_string(), last_sender_.port(),
            error.message());

        return Receive();
    }

    PacketType type = static_cast<PacketType>(receive_buffer_[0]);

    if (!packet_handle_callbacks_.contains(type))
    {
        spdlog::warn("{}:{}: Ignoring packet with unsupported type {}",
                     last_sender_.address().to_string(), last_sender_.port(),
                     static_cast<uint32_t>(type));

        return Receive();
    }

    const auto& callback = packet_handle_callbacks_.at(type);

    callback(last_sender_);

    Receive();
}
