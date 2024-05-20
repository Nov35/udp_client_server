#pragma once

#include "client_context.h"

#include <asio/ip/udp.hpp>

#include <mutex>
#include <unordered_map>

using asio::ip::udp;

class ClientStore
{
public:
    ClientStore(asio::io_context &io_context);
    ClientContext *Add(const udp::endpoint endpoint);
    ClientContext *Get(const udp::endpoint endpoint);
    bool Remove(const udp::endpoint endpoint);

private:
    using Lock = std::lock_guard<std::mutex>;

private:
    asio::io_context &io_context_;
    std::unordered_map<udp::endpoint, ClientContext::Ptr> store_;
    std::mutex store_mutex_;
};