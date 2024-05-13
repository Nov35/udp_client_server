#pragma once

#include "network.h"

class Server
{
public:
    Server(asio::io_context &io_context, uint16_t port);

private:
    ReceiveHandlingFuncs GetCallbackList();

    void TestHandle(TestPacket::Ptr packet, std::error_code error,
                    size_t bytes_received, asio::ip::udp::endpoint sender);

private:
    Network network_;
};
