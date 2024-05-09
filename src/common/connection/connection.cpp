#include "connection.h"

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

int Connection::test()
{
    asio::error_code errorcode;
    asio::ip::tcp::endpoint endpoint;

    return ASIO_VERSION;
}
