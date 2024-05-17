#include "server.h"

#include <iostream>
#include <thread>
#include <chrono>

std::mutex mut;

Server::Server(asio::io_context &io_context, uint16_t port)
    : network_(io_context, port, GetCallbackList()), pool_(std::thread::hardware_concurrency())
{
    network_.Receive();
}

ReceiveHandlingFuncs Server::GetCallbackList()
{
    ReceiveHandlingFuncs callbacks;

    callbacks.insert({PacketType::InitialRequest,
                      [this](asio::ip::udp::endpoint sender)
                      {
                          asio::post(pool_, std::bind(&Server::HandleInitialRequest, this, sender,
                                                      network_.GetPacket(std::make_shared<InitialRequest>())));
                      }});

    return callbacks;
}

void Server::HandleInitialRequest(asio::ip::udp::endpoint sender, Packet::Ptr Packet)
{
}
