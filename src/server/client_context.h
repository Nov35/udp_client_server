#pragma once

#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

#include <vector>
#include <memory>

enum class ClientState
{
    Accepted,
    InProgress,
    WaitingForPacketCheck,
    Done
};

struct DataChunk
{
    double* begin_;
    double* end_;
};

class ClientContext
{
public:
    using Ptr = std::unique_ptr<ClientContext>;

public:
    ClientContext(asio::io_context &io_context);

    ClientState GetState();
    void SetState(const ClientState state);
    
    void PrepareData(const double range);

    size_t GetIteration();
    void IncreaseIteration();

    DataChunk GetChunkOfData();

private:
    //! In real world example separate lock for each client might be needed
    // TODO Add resending of command packets by timer
    size_t iteration_;
    ClientState state_;
    asio::steady_timer timer_;
    std::vector<double> data_;
};
