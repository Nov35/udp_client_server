#include "client_context_impl.h"

#include "constants.h"
#include "data_chunk.h"

#include <algorithm>
#include <random>

ClientContextImpl::ClientContextImpl(asio::io_context &io_context)
    : iteration_(0), state_(ClientState::Accepted), timer_(io_context)
{
}

ClientState ClientContextImpl::GetState()
{
    return state_;
}

void ClientContextImpl::SetState(const ClientState state)
{
    state_ = state;
}

void ClientContextImpl::PrepareData(const double range)
{
    data_.resize(constants::values_to_transfer);

    std::random_device engine;
    std::uniform_real_distribution<double> distribution(-range, range);

    std::generate_n(data_.begin(), constants::values_to_transfer,
                    [&engine, &distribution]()
                    {
                        return distribution(engine);
                    });
}

const std::vector<double> &ClientContextImpl::GetData()
{
    return data_;
}

void ClientContextImpl::NextIteration()
{
    ++iteration_;
}

size_t ClientContextImpl::CurrentChunk()
{
    return iteration_ + 1;
}

DataChunk ClientContextImpl::GetChunkOfData()
{
    using namespace constants;

    size_t begin = iteration_ * elements_in_one_chunk;

    if (begin >= data_.size())
        return DataChunk{nullptr, nullptr, 0};

    size_t end = std::min(begin + elements_in_one_chunk, data_.size());

    size_t packets_count = (end - begin) / constants::max_payload_elements;

    if ((end - begin) % constants::max_payload_elements != 0)
        ++packets_count;

    return DataChunk{(data_.data() + begin), (data_.data() + end), packets_count};
}

asio::steady_timer &ClientContextImpl::GetTimer()
{
    return timer_;
}

std::mutex &ClientContextImpl::GetMutex()
{
    return mutex_;
}
