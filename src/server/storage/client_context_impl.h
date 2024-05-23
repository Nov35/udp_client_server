#pragma once

#include "client_state.h"

#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

#include <vector>
#include <memory>
#include <mutex>

class DataChunk;

//TODO Re-implement repeated sending with iteration check
class ClientContextImpl
{
public:
    using Ptr = std::unique_ptr<ClientContextImpl>;

public:
    ClientContextImpl(asio::io_context &io_context);

    ClientState GetState();
    void SetState(const ClientState state);

    void PrepareData(const double range);
    const std::vector<double>& GetData();

    DataChunk GetChunkOfData();
    void NextIteration();

    size_t GetCurrentIteration();

    std::mutex& GetMutex();

private:
    size_t iteration_;
    ClientState state_;
    std::vector<double> data_;
    std::mutex mutex_;
    bool is_waiting_;
};
