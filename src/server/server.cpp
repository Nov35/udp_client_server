#include "server.h"

#include "version_info.h"

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

void Server::LogAndSendError(const udp::endpoint client, const std::string_view error)
{
    spdlog::error("{}:{}: Client rejected: {}",
                  client.address().to_string(), client.port(), error);

    ServerResponse::Ptr response;
    response->is_successful_ = false;
    response->message_ = error;

    network_.Send(response, client);
}

void Server::SendDataChunck(ClientContext* context, const udp::endpoint receiver)
{
    auto [begin, end] = context->GetChunkOfData();

    size_t count = 0;
    auto payload = begin;

    while (payload != end)
    {
        PayloadMessage::Ptr packet;

        auto payload_end = std::min(
            payload + constants::max_payload_elements,
            end);
        
        packet->packet_id_ = ++count;
        packet->payload_size_ = payload_end - payload;
        payload = std::copy(payload, payload_end, packet->payload_);

        network_.Send(packet, receiver);
    }

    context->SetState(ClientState::WaitingForPacketCheck);

    PacketCheckRequest::Ptr check_request;
    check_request->packets_sent_ = count;


    network_.Send(check_request, receiver);
}

void Server::HandleInitialRequest(const InitialRequest::Ptr packet, const udp::endpoint sender)
{
    if (packet->major_version_ != project::major_version ||
        packet->minor_version_ != project::minor_version)
        return LogAndSendError(sender,
                               fmt::format("Unsupported version.\n\
                                The server version is: {}, and client's version is: {}.{}.{}",
                                           project::version_string, packet->major_version_,
                                           packet->minor_version_, packet->patch_version_));

    ClientContext *context = clients_.Add(sender);

    if (context == nullptr)
        return LogAndSendError(sender, "The client's endpoint is already accepted.");

    ServerResponse::Ptr response;
    response->is_successful_ = true;

    network_.Send(response, sender);
}

void Server::HandleRangeSettingMessage(const RangeSettingMessage::Ptr packet, const udp::endpoint sender)
{
    ClientContext *context = clients_.Get(sender);

    if (context == nullptr || context->GetState() != ClientState::Accepted)
        return LogAndSendError(sender, "Attempt to start transaction for client in incorrect state.");

    if (packet->range_constant_ < 1.0)
    {
        clients_.Remove(sender);
        return LogAndSendError(sender, "The value is too small. Please set value at least equal to 1 or greater.");
    }

    context->SetState(ClientState::InProgress);
    context->PrepareData(packet->range_constant_);

    SendDataChunck(context, sender);
}

void Server::HandlePacketCheckResponse(const PacketCheckResponse::Ptr packet, const udp::endpoint sender)
{
}
