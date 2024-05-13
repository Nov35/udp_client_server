#include "network.h"

#include<common/connection/serializer.h>

#include <asio.hpp>
#include <vector>
#include <string>

using asio::ip::udp;

constexpr size_t expected_packet_size = 1500;

Network::Network(asio::io_context &io_context,
                 ReceiveHandlingFuncs packet_handle_callbacks)
    : socket_(io_context),
      packet_handle_callbacks_(packet_handle_callbacks)
{
    socket_.open(udp::v4());
}

Network::Network(asio::io_context &io_context, uint16_t port,
                 ReceiveHandlingFuncs packet_handle_callbacks)
    : socket_(io_context, udp::endpoint(udp::v4(), port)),
      packet_handle_callbacks_(packet_handle_callbacks)
{
}

void Network::Send(Packet::Ptr packet, udp::endpoint receiver_endpoint)
{
    asio::streambuf buffer;
    Serialize(packet, buffer);
    socket_.async_send_to(buffer.data(), receiver_endpoint,
                          std::bind(&Network::HandleSend,
                                    this,
                                    asio::placeholders::error,
                                    asio::placeholders::bytes_transferred));
}

void Network::Receive()
{
    udp::endpoint sender_endpoint;
    asio::streambuf::mutable_buffers_type buffer_stream = recv_buffer_.prepare(expected_packet_size);

    socket_.async_receive_from(buffer_stream, sender_endpoint,
                               std::bind(&Network::HandleReceive,
                                         this,
                                         asio::placeholders::error,
                                         asio::placeholders::bytes_transferred,
                                         std::move(sender_endpoint)));
}

Packet::Ptr Network::GetPacket(Packet::Ptr destination)
{
    Deserialize(recv_buffer_, destination);
    return destination;
}

void Network::HandleSend(const std::error_code& , std::size_t bytes_transferred)
{
}

void Network::HandleReceive(const std::error_code& error,
                            size_t bytes_transferred,
                            udp::endpoint sender_endpoint)
{

        std::cout << "here" << std::endl;
    if (error)
    {

        return;
    }


    std::istream is(&recv_buffer_);


    PacketType type;
    is >> *(reinterpret_cast<uint8_t *>(&type));

    if (!packet_handle_callbacks_.contains(type))
    {

        return;
    }
    
    auto callback = packet_handle_callbacks_.at(type);

    // callback(error, bytes_transferred, sender_endpoint);
}
