#pragma once

#include "network.h"

#include <asio/steady_timer.hpp>

namespace asio
{
    class io_context;
}

class Client
{
public:
    Client(asio::io_context &io_context,
           const std::string server_ip,
           const uint16_t port, const double range_constant);

private:
    void start();
    ReceiveHandlingFuncs GetCallbackList();
    
    void MakeInitialRequest();
    void SendRangeSetting();

private:
    void HandleSeverResponse(const ServerResponse::Ptr packet,
                             const udp::endpoint sender);
    void HandlePacketCheckRequest(const PacketCheckRequest::Ptr packet,
                                  const udp::endpoint sender);
    void HandlePayloadMessage(const PayloadMessage::Ptr packet,
                              const udp::endpoint sender);

private:
    asio::io_context &io_context_;
    Network network_;
    const double range_constant_;
    udp::endpoint server_endpoint_;
    asio::steady_timer timer_;
};
