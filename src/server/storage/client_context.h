#pragma once

#include "repeating_timer.h"

#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

#include <vector>
#include <memory>

class DataChunk;

enum class ClientState
{
    Accepted,
    InProgress,
    WaitingForPacketCheck,
    Done
};

class ClientContext
{
public:
    using Ptr = std::unique_ptr<ClientContext>;

public:
    ClientContext(asio::io_context &io_context);

    ClientState GetState();
    void SetState(const ClientState state);

    RepeatingTimer& Repeat();
    
    void PrepareData(const double range);
    const std::vector<double>& GetData();

    void NextChunkOfData();
    size_t GetCurrentIteration();

    DataChunk GetChunkOfData();

private:
    size_t iteration_;
    ClientState state_;
    RepeatingTimer repeat_;
    std::vector<double> data_;
};
