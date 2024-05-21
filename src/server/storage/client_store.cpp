#include "client_store.h"
#include "spdlog/spdlog.h"

#include <memory>

ClientStore::ClientStore(asio::io_context &io_context) : io_context_(io_context)
{
}

ClientContext *ClientStore::Add(const udp::endpoint endpoint)
{
    Lock lock(store_mutex_);

    if (store_.contains(endpoint))
    {
        spdlog::error("Cannot insert user context for endpoint {}:{} to the store since it's already present",
                      endpoint.address().to_string(), endpoint.port());
        return nullptr;
    }

    auto [entry, result] = store_.emplace(endpoint, std::make_unique<ClientContext>(io_context_));

    return entry->second.get();
}

ClientContext *ClientStore::Get(const udp::endpoint endpoint)
{
    Lock lock(store_mutex_);

    auto entry = store_.find(endpoint);

    if (entry == store_.end())
    {
        spdlog::error("Attempt to access non-existent user context for endpoint {}:{}",
                      endpoint.address().to_string(), endpoint.port());
        return nullptr;
    }

    return entry->second.get();
}

bool ClientStore::Remove(const udp::endpoint endpoint)
{
    Lock lock(store_mutex_);

    auto entry = store_.find(endpoint);
    if (entry == store_.end())
        return false;

    store_.erase(entry);

    return true;
}
