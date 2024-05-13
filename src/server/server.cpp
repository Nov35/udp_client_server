#include "server.h"

#include <iostream>


Server::Server(asio::io_context &io_context, uint16_t port)
    : network_(io_context, port, GetCallbackList())
{
    network_.Receive();
}

ReceiveHandlingFuncs Server::GetCallbackList()
{
    ReceiveHandlingFuncs callbacks;
    
    callbacks.insert({PacketType::TEST_TYPE, 
    [this](std::error_code error, size_t bytes_received, asio::ip::udp::endpoint sender)
        {
            TestPacket::Ptr test = std::make_shared<TestPacket>();
            network_.GetPacket(test);
            TestHandle(test, error, bytes_received, sender);
        }
    });

    return callbacks;
}

//! The code is used for testing purposes and must be removed later
void Server::TestHandle(TestPacket::Ptr packet, std::error_code error, size_t bytes_received, asio::ip::udp::endpoint sender)
{
    if (!error)
    {
        std::cout << "Received message : " << packet->str_ << " Packet size: " <<  bytes_received << '\n'
        << "Sender address: " << sender.address() << ':' << sender.port() << '\n' <<
        "Sending back." << '\n';
        
        network_.Send(packet, sender);
    }
}
