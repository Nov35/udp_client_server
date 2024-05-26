#include "server.h"

#include "version_info.h"
#include "data_chunk.h"

#include <spdlog/spdlog.h>

#include <cstring>
#include <thread>

// TODO Check server methods for possible deadlock

Server::Server(asio::io_context &io_context, const uint16_t port)
    : network_(io_context, port, GetCallbackList()),
      io_context_(io_context),
      //   pool_(std::thread::hardware_concurrency()),
      clients_(io_context), timer_(io_context)
{
    network_.Receive();
}

ReceiveHandlingFuncs Server::GetCallbackList()
{
    ReceiveHandlingFuncs callbacks;

    callbacks.insert({PacketType::InitialRequest,
                      [this](udp::endpoint sender)
                      {
                          InitialRequest::Ptr packet = network_.GetReceivedPacket<InitialRequest>();
                          asio::post(io_context_.get_executor(), std::bind(&Server::HandleInitialRequest, this, packet, sender));
                      }});

    callbacks.insert({PacketType::RangeSettingMessage,
                      [this](udp::endpoint sender)
                      {
                          RangeSettingMessage::Ptr packet = network_.GetReceivedPacket<RangeSettingMessage>();

                          asio::post(io_context_.get_executor(), std::bind(&Server::HandleRangeSettingMessage,
                                                                           this, packet, sender));
                      }});

    callbacks.insert({PacketType::PacketCheckResponse,
                      [this](udp::endpoint sender)
                      {
                          PacketCheckResponse::Ptr packet = network_.GetReceivedPacket<PacketCheckResponse>();

                          asio::post(io_context_.get_executor(), std::bind(&Server::HandlePacketCheckResponse,
                                                                           this, packet, sender));
                      }});

    return callbacks;
}

void Server::SendChunkOfData(LockedContext context, const udp::endpoint client)
{
    auto chunk = context.GetChunkOfData();

    for (size_t i = 1; i <= chunk.GetPacketsCount(); ++i)
    {
        auto [begin, end] = chunk.GetPayload(i);

        PayloadMessage::Ptr packet = std::make_shared<PayloadMessage>();
        packet->packet_id_ = i;
        packet->payload_size_ = end - begin;
        std::copy(begin, end, packet->payload_);

        spdlog::trace("{}:{}: Sending packet id: {} Size: {}",
                      client.address().to_string(), client.port(), i, packet->payload_size_);

        network_.SerializeAndSend(packet, client);
    }

    SendPacketCheckRequest(std::move(context), client);
}

void Server::ResendMissingPackets(const PacketCheckResponse::Ptr response, const udp::endpoint client)
{
    LockedContext context = clients_.Get(client);

    if (context.Empty())
        throw std::logic_error("User storage was removed during data transfer");

    spdlog::warn("{}:{}: Preparing {} missing packets",
                 client.address().to_string(), client.port(), response->packets_missing_);

    auto chunk = context.GetChunkOfData();

    for (size_t i = 0; i < response->packets_missing_; ++i)
    {
        size_t requested_packet = response->missing_packets_[i];
        auto [begin, end] = chunk.GetPayload(requested_packet);

        if (begin == nullptr)
        {
            context.Unlock();
            SendErrorAndRemoveClient(client, fmt::format("Attempt to get non-existing packet: {}.",
                                                         requested_packet));
            return;
        }

        PayloadMessage::Ptr packet = std::make_shared<PayloadMessage>();
        packet->packet_id_ = requested_packet;
        packet->payload_size_ = end - begin;
        std::copy(begin, end, packet->payload_);

        spdlog::trace("{}:{}: Sending packet id: {} Size: {}",
                      client.address().to_string(), client.port(), requested_packet, packet->payload_size_);

        network_.SerializeAndSend(packet, client);
    }

    SendPacketCheckRequest(std::move(context), client);
}

void Server::SendPacketCheckRequest(LockedContext context, const udp::endpoint client)
{
    auto chunk = context.GetChunkOfData();

    PacketCheckRequest::Ptr check_request = std::make_shared<PacketCheckRequest>();
    check_request->chunk_ = context.CurrentChunk();
    check_request->packets_sent_ = chunk.GetPacketsCount();

    spdlog::info("{}:{} Sending request to check {} packets from chunk {}.",
                 client.address().to_string(), client.port(), check_request->packets_sent_,
                 check_request->chunk_);

    context.SetState(ClientState::WaitingForResponse);

    context.Unlock();

    network_.SerializeAndSend(check_request, client);

    if (check_request->packets_sent_ == 0)
    {
        spdlog::info("{}:{} All packets were successfully received by client.\n",
                     client.address().to_string(), client.port());
        clients_.Remove(client);
    }
}

// void Server::RepeatedlySend(const CommandPacket::Ptr packet, const udp::endpoint client,
//                             asio::chrono::milliseconds delay)
// {
//     LockedContext context = clients_.Get(client);

//     spdlog::info("enter chunk{}, delay = {}" ,packet->chunk_, delay.count());

//     if (context.Empty())
//         return;

//     if (packet->chunk_ < context.CurrentChunk() ||
//         context.GetState() != ClientState::WaitingForResponse)
//         return;

//     if (delay > asio::chrono::milliseconds(50000))
//     {
//         context.Unlock();
//         SendErrorAndRemoveClient(client, fmt::format("Timeout exceeded. No response from the client for chunk: {}.", packet->chunk_));
//         return;
//     }

//     network_.SerializeAndSend(packet, client);

//     timer_.expires_from_now(asio::chrono::milliseconds(2000));
//     timer_.async_wait(std::bind(&Server::RepeatedlySend,
//                                 this, packet, client, delay * 10));
// }

void Server::HandleInitialRequest(const InitialRequest::Ptr request, const udp::endpoint client)
{
    if (request->major_version_ == 0 &&
        request->minor_version_ == 0 &&
        request->patch_version_ == 0)
    {
        spdlog::warn("Ignorring corrupted init request packet");
        return;
    }

    if (request->major_version_ != project::major_version ||
        request->minor_version_ != project::minor_version)
    {
        SendErrorAndRemoveClient(client,
                                 fmt::format("Unsupported version.\n\
                                The server version is: {}, and client's version is: {}.{}.{}",
                                             project::version_string, request->major_version_,
                                             request->minor_version_, request->patch_version_));
        return;
    }

    LockedContext context = clients_.Add(client);

    if (context.Empty())
    {
        spdlog::warn("{}:{}: Duplicate initialization request",
                     client.address().to_string(), client.port());
        return;
    }

    spdlog::info("{}:{} Accepted",
                 client.address().to_string(), client.port());

    ServerResponse::Ptr response = std::make_shared<ServerResponse>();
    response->chunk_ = 1;
    response->is_successful_ = true;

    network_.SerializeAndSend(response, client);
}

void Server::HandleRangeSettingMessage(const RangeSettingMessage::Ptr setting, const udp::endpoint client)
{
    LockedContext context = clients_.Get(client);

    if (context.Empty())
        return SendErrorAndRemoveClient(client, "Attempt to start transaction for client in incorrect state.");

    if (context.GetState() != ClientState::Accepted)
    {
        spdlog::warn("Ignorring duplicate range setting");
        return;
    }

    if (setting->range_constant_ < 1.0)
    {
        context.Unlock();
        return SendErrorAndRemoveClient(client, "The value is too small. Please set value at least equal to 1 or greater.");
    }

    spdlog::info("{}:{} Received range constant: {}",
                 client.address().to_string(), client.port(), setting->range_constant_);

    context.SetState(ClientState::InProgress);
    context.PrepareData(setting->range_constant_);

    SendChunkOfData(std::move(context), client);
}

void Server::HandlePacketCheckResponse(const PacketCheckResponse::Ptr response, const udp::endpoint client)
{
    LockedContext context = clients_.Get(client);

    if (context.Empty())
    {
        spdlog::warn("Check request for non-existing client.");
        return;
    }

    if (context.GetState() != ClientState::WaitingForResponse || response->chunk_ < context.CurrentChunk())
    {
        spdlog::warn("Ignorring out of order or duplicate check response \n\
         Response: {}, Actual: {}",
                     response->chunk_, context.CurrentChunk());
        return;
    }

    context.SetState(ClientState::InProgress);

    if (response->packets_missing_ == 0)
    {
        spdlog::info("Sending next chunk of data");

        context.NextIteration();
        return SendChunkOfData(std::move(context), client);
    }

    context.Unlock();
    ResendMissingPackets(response, client);
}

void Server::SendErrorAndRemoveClient(const udp::endpoint client, const std::string_view error)
{
    asio::post(io_context_.get_executor(), std::bind(&ClientStore::Remove, &clients_, client));

    spdlog::error("{}:{}: Client rejected: {}",
                  client.address().to_string(), client.port(), error);

    ServerResponse::Ptr response = std::make_shared<ServerResponse>();
    response->is_successful_ = false;
    response->msg_lenght_ = error.length();
    std::strcpy(response->message_, error.data());

    network_.SerializeAndSend(response, client);
}
