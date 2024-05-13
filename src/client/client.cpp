#include "client.h"

#include <asio.hpp>

#include <string>
#include <functional>

using asio::ip::udp;

Client::Client(asio::io_context &io_context, std::string server_ip, uint16_t port)
    : network_(io_context, GetCallbackList())
{
    udp::resolver resolver(io_context);

    server_endpoint_ =
        *resolver.resolve(udp::v4(), server_ip, std::to_string(port)).begin();


    //!TEST

    TestPacket::Ptr test = std::make_shared<TestPacket>();
    test->str_ = "Just a string.";
    network_.Send(test, server_endpoint_);
    network_.Receive();
}

ReceiveHandlingFuncs Client::GetCallbackList()
{
    ReceiveHandlingFuncs callbacks;
    
    return callbacks;
}
