#include "data_chunk.h"

#include "constants.h"
#include <algorithm>

DataChunk::DataChunk(const double *begin, const double *end, const size_t count)
    : begin_(begin), end_(end), packets_count_(count)
{
}

const size_t DataChunk::GetPacketsCount()
{
    return packets_count_;
}

DataChunk::Payload DataChunk::GetPayload(const size_t packet_num)
{
    using namespace constants;

    if (packet_num > packets_count_ || packet_num < 1)
        return {nullptr, nullptr};

    const double *payload_begin = begin_ + max_payload_elements * (packet_num - 1);
    const double *payload_end = std::min(payload_begin + max_payload_elements, end_);

    return {payload_begin, payload_end};
}
