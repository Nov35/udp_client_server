#include "network.h"

#include "serializer.h"

#include <asio.hpp>
#include <vector>
#include <string>

using asio::ip::udp;

Network::Network(asio::io_context &io_context,
                 ReceiveHandlingFuncs packet_handle_callbacks)
    : socket_(io_context, udp::v4()), receive_buffer_(constants::expected_packet_size),
      packet_handle_callbacks_(packet_handle_callbacks)
{
}

Network::Network(asio::io_context &io_context, uint16_t port,
                 ReceiveHandlingFuncs packet_handle_callbacks)
    : socket_(io_context, udp::endpoint(udp::v4(), port)),
      receive_buffer_(constants::expected_packet_size),
      packet_handle_callbacks_(packet_handle_callbacks)
{
}

void Network::Send(Packet::Ptr packet, udp::endpoint receiver_endpoint)
{
    BinaryData buffer;
    size_t data_size = Serialize(packet, buffer);
    socket_.async_send_to(asio::buffer(buffer, data_size), receiver_endpoint,
                          std::bind(&Network::HandleSend,
                                    this,
                                    asio::placeholders::error,
                                    asio::placeholders::bytes_transferred));
}

void Network::Receive()
{
    socket_.async_receive_from(asio::buffer(receive_buffer_), last_sender_,
                               std::bind(&Network::HandleReceive,
                                         this,
                                         asio::placeholders::error,
                                         asio::placeholders::bytes_transferred));
}

Packet::Ptr Network::GetPacket(Packet::Ptr destination)
{
    Deserialize(receive_buffer_, destination);
    return destination;
}

void Network::HandleSend(const std::error_code &, std::size_t bytes_transferred)
{
}

void Network::HandleReceive(const std::error_code &error,
                            size_t bytes_transferred)
{

    if (error)
    {

        return;
    }

    PacketType type = static_cast<PacketType>(receive_buffer_[0]);

    if (!packet_handle_callbacks_.contains(type))
    {

        return;
    }

    auto callback = packet_handle_callbacks_.at(type);

    callback(last_sender_);

    Receive();
}
