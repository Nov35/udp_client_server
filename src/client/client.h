#pragma once

#include "network.h"

#include <string>

namespace asio
{
    class io_context;
}

class Client
{
public:
    Client(asio::io_context &io_context,
           std::string server_ip,
           uint16_t port);

private:
    ReceiveHandlingFuncs GetCallbackList();

private:
    Network network_;
    asio::ip::udp::endpoint server_endpoint_;
};
