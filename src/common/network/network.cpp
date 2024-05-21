#include "network.h"

#include "serializer.h"

#include <asio.hpp>
#include <spdlog/spdlog.h>

#include <vector>

using asio::ip::udp;

Network::Network(asio::io_context &io_context,
                 ReceiveHandlingFuncs &&packet_handle_callbacks)
    : socket_(io_context), receive_buffer_(constants::max_packet_size),
      packet_handle_callbacks_(packet_handle_callbacks)
{
    socket_.open(udp::v4());
    // socket_.set_option(asio::socket_base::receive_buffer_size(1000000));
}

Network::Network(asio::io_context &io_context, const uint16_t port,
                 ReceiveHandlingFuncs &&packet_handle_callbacks)
    : socket_(io_context, udp::endpoint(udp::v4(), port)),
      receive_buffer_(constants::max_packet_size),
      packet_handle_callbacks_(packet_handle_callbacks)
{
    // socket_.set_option(asio::socket_base::receive_buffer_size(1000000));
}

void Network::Send(const Packet::Ptr packet, const udp::endpoint receiver_endpoint)
{
    BinaryData buffer;
    size_t data_size = Serialize(packet, buffer);
    socket_.async_send_to(asio::buffer(buffer, data_size), receiver_endpoint,
                          std::bind(&Network::HandleSend,
                                    this,
                                    asio::placeholders::error,
                                    asio::placeholders::bytes_transferred,
                                    receiver_endpoint));
}

void Network::SendRepeatedly(const Packet::Ptr packet, const udp::endpoint receiver_endpoint,
                             asio::steady_timer &timer, FailCallbackFunc fail_callback, size_t count)
{
    Send(packet, receiver_endpoint);

    if (count < constants::send_attempts)
    {
        timer.expires_from_now(asio::chrono::milliseconds(500));
        timer.async_wait(std::bind(
            &Network::SendRepeatedly, this, packet, receiver_endpoint,
            std::ref(timer), fail_callback, ++count));
    }
    else
        fail_callback();
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
        spdlog::error("Error during receiving: {}", error.message());
        return;
    }

    PacketType type = static_cast<PacketType>(receive_buffer_[0]);

    if (!packet_handle_callbacks_.contains(type))
    {
        spdlog::warn("{}:{}: Ignoring packet with unsupported type {}",
                     last_sender_.address().to_string(), last_sender_.port(),
                     static_cast<uint>(type));

        return Receive();
    }

    auto callback = packet_handle_callbacks_.at(type);

    callback(last_sender_);

    Receive();
}
