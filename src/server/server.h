#pragma once

#include "network.h"
#include "client_store.h"

#include <asio/ip/udp.hpp>
#include <asio/thread_pool.hpp>

class Server
{
public:
    Server(asio::io_context &io_context, const uint16_t port);

private:
    ReceiveHandlingFuncs GetCallbackList();

    void LogAndSendError(udp::endpoint client, std::string_view error);
    void SendDataChunck(ClientContext *context, const udp::endpoint receiver);

private:
    void HandleInitialRequest(const InitialRequest::Ptr packet,
                              const udp::endpoint sender);
    void HandleRangeSettingMessage(const RangeSettingMessage::Ptr packet,
                                   const udp::endpoint sender);
    void HandlePacketCheckResponse(const PacketCheckResponse::Ptr packet,
                                   const udp::endpoint sender);

private:
    Network network_;
    asio::thread_pool pool_;
    ClientStore clients_;
};
