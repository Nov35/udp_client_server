#pragma once

#include "packet_types.h"

#include <asio/ip/udp.hpp>
#include <asio/steady_timer.hpp>

#include <functional>
#include <unordered_map>
#include <mutex>

using asio::ip::udp;

using ReceiveHandleFunc = std::function<void(udp::endpoint)>;
using ReceiveHandlingFuncs = std::unordered_map<PacketType, ReceiveHandleFunc>;

class Network
{
public:
    Network(asio::io_context &io_context, ReceiveHandlingFuncs &&packet_handle_callbacks);
    Network(asio::io_context &io_context, const uint16_t port, ReceiveHandlingFuncs &&packet_handle_callbacks);

    void Send(const Packet::Ptr packet, const udp::endpoint receiver_endpoint);
    void Receive();
    void GetPacket(Packet::Ptr destination);

private:
    void HandleSend(const std::error_code &error,
                    size_t bytes_transferred, const udp::endpoint receiver_endpoint);
    void HandleReceive(const std::error_code &error,
                       const size_t bytes_received);

private:
    udp::socket socket_;
    BinaryData receive_buffer_;
    const ReceiveHandlingFuncs packet_handle_callbacks_;
    udp::endpoint last_sender_;
};
