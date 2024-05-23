#pragma once

#include "network.h"
#include "storage/client_store.h"

#include <asio/ip/udp.hpp>
#include <asio/steady_timer.hpp>
#include <asio/thread_pool.hpp>

class Server
{
public:
    Server(asio::io_context &io_context, const uint16_t port);

private:
    ReceiveHandlingFuncs GetCallbackList();

    void SendChunkOfData(LockedContext context, const udp::endpoint client);
    void SendPacketCheckRequest(LockedContext context, const udp::endpoint client);
    void RepeatedlySend(const CommandPacket::Ptr packet, const udp::endpoint client,
                        asio::chrono::milliseconds delay = asio::chrono::milliseconds(50));
    void ResendMissingPackets(const PacketCheckResponse::Ptr response,
                              const udp::endpoint client);
    void SendErrorAndRemoveClient(udp::endpoint client, std::string_view error);

private:
    void HandleInitialRequest(const InitialRequest::Ptr request,
                              const udp::endpoint client);
    void HandleRangeSettingMessage(const RangeSettingMessage::Ptr setting,
                                   const udp::endpoint client);
    void HandlePacketCheckResponse(const PacketCheckResponse::Ptr response,
                                   const udp::endpoint client);

private:
    Network network_;
    asio::thread_pool pool_;
    asio::steady_timer timer_;
    ClientStore clients_;
};
