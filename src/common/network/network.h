#pragma once

#include <packet_types.h>

#include <asio/ip/udp.hpp>
#include <asio/streambuf.hpp>

#include <functional>
#include <mutex>
#include <unordered_map>

using ReceiveHandleFunc = std::function<void(std::error_code, size_t, asio::ip::udp::endpoint)>;
using ReceiveHandlingFuncs = std::unordered_map<PacketType, ReceiveHandleFunc>;

class Network
{
public:
    Network(asio::io_context &io_context, ReceiveHandlingFuncs packet_handle_callbacks);
    Network(asio::io_context &io_context, uint16_t port, ReceiveHandlingFuncs packet_handle_callbacks);

    void Send(Packet::Ptr packet, asio::ip::udp::endpoint receiver_endpoint);
    void Receive();
    Packet::Ptr GetPacket(Packet::Ptr destination);

private:
    void HandleSend(const std::error_code& error,
                    std::size_t bytes_transferred);
                    
    void HandleReceive(const std::error_code&  error,
                       size_t bytes_transferred,
                       asio::ip::udp::endpoint sender_endpoint);

private:
    asio::ip::udp::socket socket_;
    const ReceiveHandlingFuncs packet_handle_callbacks_;
    asio::streambuf recv_buffer_;
    //TODO add lock and/or queue for packets
};
