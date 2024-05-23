#pragma once

#include "client_state.h"

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

    bool Empty();
    void Unlock();

private:
    ClientContextImpl *context_ptr_;
    std::unique_lock<std::mutex> lock_;
};
