#include "client.h"

#include <array>
#include <iostream>
#include <asio.hpp>
#include <thread>

using asio::ip::udp;

int main()
{
    try
    {
        asio::io_context io_context;

        Client cl(io_context, "127.0.0.1", 5555);
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
