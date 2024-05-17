#pragma once

#include "network.h"

#include <asio/thread_pool.hpp>
#include <asio/ip/udp.hpp>

class Server
{
public:
    Server(asio::io_context &io_context, uint16_t port);

private:
    ReceiveHandlingFuncs GetCallbackList();

    void HandleInitialRequest(asio::ip::udp::endpoint sender, Packet::Ptr Packet);

private:
    Network network_;
    asio::thread_pool pool_;
};
