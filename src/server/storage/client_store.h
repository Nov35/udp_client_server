#pragma once

#include "locked_context.h"

#include <asio/io_context.hpp>
#include <asio/ip/udp.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>


using asio::ip::udp;
class ClientContextImpl;

class ClientStore
{
public:
    ClientStore(asio::io_context &io_context);
    ~ClientStore();
    LockedContext Add(const udp::endpoint endpoint);
    LockedContext Get(const udp::endpoint endpoint);
    void Remove(const udp::endpoint endpoint);

private:
    using Lock = std::lock_guard<std::mutex>;

private:
    asio::io_context &io_context_;
    std::unordered_map<udp::endpoint, std::unique_ptr<ClientContextImpl>> store_;
    std::mutex store_mutex_;
};
