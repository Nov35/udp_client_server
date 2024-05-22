#include "client_context.h"

#include "constants.h"
#include "data_chunk.h"

#include <algorithm>
#include <random>

ClientContext::ClientContext(asio::io_context &io_context)
    : iteration_(0), state_(ClientState::Accepted)
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

    std::random_device engine;
    std::uniform_real_distribution<double> distribution(-range, range);

    std::generate_n(data_.begin(), constants::values_to_transfer,
                    [&engine, &distribution]()
                    {
                        return distribution(engine);
                    });
}

const std::vector<double> &ClientContext::GetData()
{
    return data_;
}

void ClientContext::NextChunkOfData()
{
    ++iteration_;
}

size_t ClientContext::GetCurrentIteration()
{
    return iteration_;
}

DataChunk ClientContext::GetChunkOfData()
{
    using namespace constants;

    size_t begin = iteration_ * elements_in_one_chunk;

    if(begin >= data_.size())
        return DataChunk{nullptr, nullptr, 0};

    size_t end = std::min(begin + elements_in_one_chunk, data_.size());

    size_t packets_count = (end - begin) / constants::max_payload_elements;

    if((end - begin) % constants::max_payload_elements != 0)
        ++packets_count;

    return DataChunk{(data_.data() + begin), (data_.data() + end), packets_count};
}
