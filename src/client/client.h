#pragma once

#include "network.h"

#include <mutex>

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

    void start();
    
private:
    ReceiveHandlingFuncs GetCallbackList();

    void MakeInitialRequest();
    void SendRangeSetting();
    void FlushBuffer();

private:
    void HandleSeverResponse(const ServerResponse::Ptr response,
                             const udp::endpoint sender);
    void HandlePayloadMessage(const PayloadMessage::Ptr packet,
                              const udp::endpoint sender);
    void HandlePacketCheckRequest(const PacketCheckRequest::Ptr request,
                                  const udp::endpoint sender);
    void ProcessData();

private:
    using Lock = std::scoped_lock<std::mutex>;

private:
    asio::io_context &io_context_;
    Network network_;
    udp::endpoint server_endpoint_;

    const double range_constant_;
    asio::steady_timer first_delay_timer_;

    size_t chunks_collected_;
    std::vector<PayloadMessage::Ptr> buffer_;
    std::vector<double> collected_data_;
    std::mutex data_mutex_;
};
