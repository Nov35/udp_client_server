#include "server.h"

#include <asio.hpp>
#include <iostream>

using asio::ip::udp;

int main()
{
  try
  {
    asio::io_context io_context;
    Server server(io_context, 5555);
    io_context.run();
  }
  catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
