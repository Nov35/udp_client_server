#pragma once

#include "packet_types.h"
#include "serializer.h"

#include <asio/ip/udp.hpp>
#include <asio/steady_timer.hpp>

#include <functional>
#include <mutex>
#include <unordered_map>

using asio::ip::udp;

using ReceiveHandleFunc = std::function<void(udp::endpoint)>;
using ReceiveHandlingFuncs = std::unordered_map<PacketType, ReceiveHandleFunc>;

class Network
{
public:
    Network(asio::io_context &io_context, ReceiveHandlingFuncs &&packet_handle_callbacks, const std::string server_ip,
            const uint16_t port);
    Network(asio::io_context &io_context, const uint16_t port, ReceiveHandlingFuncs &&packet_handle_callbacks);
    void Receive();

public:
    template <class PacketPtr>
    void SerializeAndSend(const PacketPtr packet, const udp::endpoint receiver_endpoint)
    {
        size_t data_size = Serialize<decltype(*packet)>(*packet, send_buffer_);
        SendFromBuffer(data_size, receiver_endpoint);
    }

    template <class PacketType>
    auto GetReceivedPacket()
    {
        auto packet = std::make_shared<PacketType>();
        Deserialize<PacketType>({receive_buffer_, bytes_received_}, *packet);
        return packet;
    }

private:
    void SendFromBuffer(size_t data_size, const udp::endpoint receiver_endpoint);
    void HandleSend(const std::error_code &error,
                    size_t bytes_transferred, const udp::endpoint receiver_endpoint);
    void HandleReceive(const std::error_code &error,
                       const size_t bytes_received);

private:
    udp::socket socket_;
    BinaryData send_buffer_;
    BinaryData receive_buffer_;
    size_t bytes_received_;
    const ReceiveHandlingFuncs packet_handle_callbacks_;
    udp::endpoint last_sender_;
};
