#pragma once

#include "network.h"
#include "storage/client_store.h"

#include <asio/ip/udp.hpp>
#include <spdlog/fmt/fmt.h>

class Server
{
public:
    Server(asio::io_context &io_context, const uint16_t port);

private:
    ReceiveHandlingFuncs GetCallbackList();

    void SendChunkOfData(LockedContext context, const udp::endpoint client);
    void SendPacketCheckRequest(LockedContext context, const udp::endpoint client);
    void ResendMissingPackets(const PacketCheckResponse::Ptr response,
                              const udp::endpoint client);
    void SendErrorAndRemoveClient(udp::endpoint client, std::string_view error);

private:
    template <class PacketPtr>
    void SendRepeatedly(PacketPtr packet, const udp::endpoint client, size_t count = 0,
                        std::error_code = std::error_code{});

private:
    void HandleInitialRequest(const InitialRequest::Ptr request,
                              const udp::endpoint client);
    void HandleRangeSettingMessage(const RangeSettingMessage::Ptr setting,
                                   const udp::endpoint client);
    void HandlePacketCheckResponse(const PacketCheckResponse::Ptr response,
                                   const udp::endpoint client);

private:
    Network network_;
    asio::io_context &io_context_;
    ClientStore clients_;
};

template <class PacketPtr>
inline void Server::SendRepeatedly(PacketPtr packet, const udp::endpoint client,
                                   size_t count, std::error_code error)
{
    if (error == asio::error::operation_aborted)
        return;

    LockedContext context = clients_.Get(client);

    if (context.Empty())
        return;

    if (packet->chunk_ < context.CurrentChunk() ||
        context.GetState() != ClientState::WaitingForResponse)
        return;

    if (count > constants::send_attempts)
    {
        context.Unlock();
        SendErrorAndRemoveClient(client, fmt::format("Timeout exceeded. No response from the client."));
        return;
    }

    network_.SerializeAndSend(packet, client);

    auto& timer = context.GetTimer();
    timer.expires_from_now(asio::chrono::milliseconds(constants::resend_delay_ms));
    timer.async_wait(std::bind(&Server::SendRepeatedly<PacketPtr>,
                                this, packet, client, count + 1, std::placeholders::_1));
}
