#pragma once

#include "client_state.h"

#include <asio/steady_timer.hpp>

#include <mutex>
#include <vector>

class ClientContextImpl;
class DataChunk;

class LockedContext
{
public:
    LockedContext();
    LockedContext(ClientContextImpl *ptr);

    ClientState GetState();
    void SetState(const ClientState state);

    void PrepareData(const double range);

    DataChunk GetChunkOfData();
    void NextIteration();
    size_t CurrentChunk();

    asio::steady_timer& GetTimer();

    bool Empty();
    void Unlock();

private:
    ClientContextImpl *impl_ptr_;
    std::unique_lock<std::mutex> lock_;
};
