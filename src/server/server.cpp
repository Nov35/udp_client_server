#include "server.h"

#include "version_info.h"
#include "data_chunk.h"

#include <spdlog/spdlog.h>

#include <thread>

Server::Server(asio::io_context &io_context, const uint16_t port)
    : network_(io_context, port, GetCallbackList()),
      pool_(std::thread::hardware_concurrency()),
      clients_(io_context)
{
    network_.Receive();
}

ReceiveHandlingFuncs Server::GetCallbackList()
{
    ReceiveHandlingFuncs callbacks;

    callbacks.insert({PacketType::InitialRequest,
                      [this](udp::endpoint sender)
                      {
                          InitialRequest::Ptr packet = std::make_shared<InitialRequest>();
                          network_.GetPacket(packet);

                          asio::post(pool_, std::bind(&Server::HandleInitialRequest,
                                                      this, packet, sender));
                      }});

    callbacks.insert({PacketType::RangeSettingMessage,
                      [this](udp::endpoint sender)
                      {
                          RangeSettingMessage::Ptr packet = std::make_shared<RangeSettingMessage>();
                          network_.GetPacket(packet);

                          asio::post(pool_, std::bind(&Server::HandleRangeSettingMessage,
                                                      this, packet, sender));
                      }});

    callbacks.insert({PacketType::PacketCheckResponse,
                      [this](udp::endpoint sender)
                      {
                          PacketCheckResponse::Ptr packet = std::make_shared<PacketCheckResponse>();
                          network_.GetPacket(packet);

                          asio::post(pool_, std::bind(&Server::HandlePacketCheckResponse,
                                                      this, packet, sender));
                      }});

    return callbacks;
}

void Server::SendErrorAndRemoveClient(const udp::endpoint client, const std::string_view error)
{
    spdlog::error("{}:{}: Client rejected: {}",
                  client.address().to_string(), client.port(), error);

    clients_.Remove(client);

    ServerResponse::Ptr response = std::make_shared<ServerResponse>();
    response->is_successful_ = false;
    response->message_ = error;

    network_.Send(response, client);
}

void Server::SendChunkOfData(ClientContext *context, const udp::endpoint receiver)
{
    auto chunk = context->GetChunkOfData();

    for (size_t i = 1; i <= chunk.GetPacketsCount(); ++i)
    {
        auto [begin, end] = chunk.GetPayload(i);

        PayloadMessage::Ptr packet = std::make_shared<PayloadMessage>();
        packet->packet_id_ = i;
        packet->payload_size_ = end - begin;
        std::copy(begin, end, packet->payload_);

        spdlog::trace("{}:{}: Sending packet id: {} Size: {}",
                      receiver.address().to_string(), receiver.port(), i, packet->payload_size_);

        network_.Send(packet, receiver);
    }

    SendPacketCheckRequest(context, receiver);
}

void Server::ResendMissingPackets(const PacketCheckResponse::Ptr packet, const udp::endpoint client)
{
    ClientContext *context = clients_.Get(client);

    if (context == nullptr)
        throw std::logic_error("User storage was removed during data transfer");

    spdlog::warn("{}:{}: Preparing {} missing packets",
                 client.address().to_string(), client.port(), packet->packets_missing_);

    auto chunk = context->GetChunkOfData();

    for (size_t i = 0; i < packet->packets_missing_; ++i)
    {
        size_t requested_packet = packet->missing_packets_[i];
        auto [begin, end] = chunk.GetPayload(requested_packet);

        if (begin == nullptr)
            return SendErrorAndRemoveClient(client, fmt::format("Attempt to get non-existing packet: {}.",
                                                                requested_packet));

        PayloadMessage::Ptr packet = std::make_shared<PayloadMessage>();
        packet->packet_id_ = requested_packet;
        packet->payload_size_ = end - begin;
        std::copy(begin, end, packet->payload_);

        spdlog::trace("{}:{}: Sending packet id: {} Size: {}",
                      client.address().to_string(), client.port(), requested_packet, packet->payload_size_);

        context->SetState(ClientState::WaitingForPacketCheck);
        network_.Send(packet, client);
    }

    SendPacketCheckRequest(context, client);
}

void Server::SendPacketCheckRequest(ClientContext *context, const udp::endpoint client)
{
    auto chunk = context->GetChunkOfData();

    PacketCheckRequest::Ptr check_request = std::make_shared<PacketCheckRequest>();
    check_request->packets_sent_ = chunk.GetPacketsCount();

    if (check_request->packets_sent_ == 0)
        clients_.Remove(client);

    spdlog::info("{}:{} Sending request to check {} packets",
                 client.address().to_string(), client.port(), check_request->packets_sent_);

    context->SetState(ClientState::WaitingForPacketCheck);
    network_.Send(check_request, client);
    // context->Repeat()(std::bind(&Network::Send, &network_, check_request, client));
}

void Server::HandleInitialRequest(const InitialRequest::Ptr packet, const udp::endpoint client)
{
    if (packet->major_version_ != project::major_version ||
        packet->minor_version_ != project::minor_version)
    {
        SendErrorAndRemoveClient(client,
                                 fmt::format("Unsupported version.\n\
                                The server version is: {}, and client's version is: {}.{}.{}",
                                             project::version_string, packet->major_version_,
                                             packet->minor_version_, packet->patch_version_));
        return;
    }

    ClientContext *context = clients_.Add(client);

    if (context == nullptr)
    {
        spdlog::warn("{}:{}: Duplicate initialization request",
                     client.address().to_string(), client.port());
        return;
    }

    // context->Repeat().SetCallback([this, client]()
    //                               { SendErrorAndRemoveClient(client, "Lost connetion. Removing client from storage."); });

    spdlog::info("{}:{} Accepted",
                 client.address().to_string(), client.port());

    ServerResponse::Ptr response = std::make_shared<ServerResponse>();
    response->is_successful_ = true;

    network_.Send(response, client);
}

void Server::HandleRangeSettingMessage(const RangeSettingMessage::Ptr packet, const udp::endpoint client)
{
    ClientContext *context = clients_.Get(client);

    if (context == nullptr)
        return SendErrorAndRemoveClient(client, "Attempt to start transaction for client in incorrect state.");

    if (context->GetState() != ClientState::Accepted)
    {
        spdlog::warn("Ignorring out of order or duplicate packet");
        return;
    }

    if (packet->range_constant_ < 1.0)
    {
        return SendErrorAndRemoveClient(client, "The value is too small. Please set value at least equal to 1 or greater.");
    }

    spdlog::info("{}:{} Received range constant: {}",
                 client.address().to_string(), client.port(), packet->range_constant_);

    context->SetState(ClientState::InProgress);
    context->PrepareData(packet->range_constant_);

    SendChunkOfData(context, client);
}

void Server::HandlePacketCheckResponse(const PacketCheckResponse::Ptr packet, const udp::endpoint client)
{
    
    ClientContext *context = clients_.Get(client);

    if (context == nullptr)
    {
        SendErrorAndRemoveClient(client, "Packet check was not requested.");
        return;
    }

    context->Repeat().stop();

    if (context->GetState() != ClientState::WaitingForPacketCheck)
    {
        spdlog::warn("Ignorring out of order or duplicate packet");
        return;
    }

    context->SetState(ClientState::InProgress);

    if (packet->packets_missing_ == 0)
    {
        spdlog::info("Sending next chunk of data");

        context->NextChunkOfData();
        return SendChunkOfData(context, client);
    }

    ResendMissingPackets(packet, client);
}
