#pragma once

#include "network.h"
#include "storage/client_store.h"

#include <asio/ip/udp.hpp>
#include <asio/thread_pool.hpp>

class Server
{
public:
    Server(asio::io_context &io_context, const uint16_t port);

private:
    ReceiveHandlingFuncs GetCallbackList();

    void SendErrorAndRemoveClient(udp::endpoint client, std::string_view error);
    void SendChunkOfData(ClientContext *context, const udp::endpoint receiver);
    void ResendMissingPackets(const PacketCheckResponse::Ptr packet,
                              const udp::endpoint client);
    void SendPacketCheckRequest(ClientContext *context, const udp::endpoint client);

private:
    void HandleInitialRequest(const InitialRequest::Ptr packet,
                              const udp::endpoint client);
    void HandleRangeSettingMessage(const RangeSettingMessage::Ptr packet,
                                   const udp::endpoint client);
    void HandlePacketCheckResponse(const PacketCheckResponse::Ptr packet,
                                   const udp::endpoint client);

private:
    Network network_;
    asio::thread_pool pool_;
    ClientStore clients_;
};
