#include "client_context.h"

#include "constants.h"
#include <algorithm>
#include <random>

ClientContext::ClientContext(asio::io_context &io_context)
    : iteration_(0), state_(ClientState::Accepted), timer_(io_context)
{
}

ClientState ClientContext::GetState()
{
    return state_;
}

void ClientContext::SetState(const ClientState state)
{
    state_ = state;
}

void ClientContext::PrepareData(const double range)
{
    data_.resize(constants::values_to_transfer);

    std::default_random_engine engine;
    std::uniform_real_distribution<double> distribution(-range, range);

    std::generate_n(data_.begin(), constants::values_to_transfer,
                    [&engine, &distribution]()
                    {
                        return distribution(engine);
                    });
}

size_t ClientContext::GetIteration()
{
    return iteration_;
}

void ClientContext::IncreaseIteration()
{
    ++iteration_;
}

DataChunk ClientContext::GetChunkOfData()
{
    using namespace constants;

    size_t begin = iteration_ * packets_chunk_size;

    if(begin >= data_.size())
        return {nullptr, nullptr};

    size_t end = std::min(begin + iteration_size, data_.size());

    return {data_.data() + begin, data_.data() + end};
}
