#include "client.h"

#include "version_info.h"

#include <asio/io_context.hpp>
#include <spdlog/spdlog.h>

#include <functional>

Client::Client(asio::io_context &io_context, const std::string server_ip,
               const uint16_t port, const double range_constant)
    : io_context_(io_context),
      network_(io_context, GetCallbackList()), 
      range_constant_(range_constant), timer_(io_context)
{
    udp::resolver resolver(io_context);

    server_endpoint_ =
        *resolver.resolve(udp::v4(), server_ip, std::to_string(port)).begin();
}

void Client::start()
{
    MakeInitialRequest();
    network_.Receive();
}

ReceiveHandlingFuncs Client::GetCallbackList()
{
    ReceiveHandlingFuncs callbacks;

    callbacks.insert({PacketType::ServerResponse,
                      [this](udp::endpoint sender)
                      {
                          ServerResponse::Ptr packet = std::make_shared<ServerResponse>();
                          network_.GetPacket(packet);

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandleSeverResponse,
                                               this, packet, sender));
                      }});

    callbacks.insert({PacketType::PacketCheckRequest,
                      [this](udp::endpoint sender)
                      {
                          PacketCheckRequest::Ptr packet = std::make_shared<PacketCheckRequest>();
                          network_.GetPacket(packet);

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandlePacketCheckRequest,
                                               this, packet, sender));
                      }});

    callbacks.insert({PacketType::PayloadMessage,
                      [this](udp::endpoint sender)
                      {
                          PayloadMessage::Ptr packet = std::make_shared<PayloadMessage>();
                          network_.GetPacket(packet);

                          asio::post(io_context_.get_executor(),
                                     std::bind(&Client::HandlePayloadMessage,
                                               this, packet, sender));
                      }});

    return callbacks;
}

void Client::MakeInitialRequest()
{
    InitialRequest::Ptr request = std::make_shared<InitialRequest>();
    request->major_version_ = project::major_version;
    request->minor_version_ = project::minor_version;
    request->patch_version_ = project::patch_version;

    network_.Send(request, server_endpoint_);
}

void Client::SendRangeSetting()
{
    RangeSettingMessage::Ptr setting;
    setting->range_constant_ = range_constant_;
    network_.Send(setting, server_endpoint_);
}

void Client::HandleSeverResponse(const ServerResponse::Ptr packet, const udp::endpoint sender)
{
    //TODO Separate initialization and error handling logic
    if (!packet->is_successful_)
    {
        spdlog::error("Rejected by server: {}\nStopping.", packet->message_);
        io_context_.stop();
        return;
    }

    timer_.expires_from_now(asio::chrono::seconds(3));
    timer_.async_wait(std::bind(&Client::SendRangeSetting, this));
}

void Client::HandlePacketCheckRequest(const PacketCheckRequest::Ptr packet, const udp::endpoint sender)
{
}

void Client::HandlePayloadMessage(const PayloadMessage::Ptr packet, const udp::endpoint sender)
{
}
