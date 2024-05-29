#include "data_chunk.h"

#include "constants.h"
#include <algorithm>

DataChunk::DataChunk(Payload data)
    : data_(data)
{
    using namespace constants;

    packets_count_ = data.size() / max_packet_payload_elements;

    if ((data.size() ) % max_packet_payload_elements != 0)
        ++packets_count_;
}

const size_t DataChunk::GetPacketsCount()
{
    return packets_count_;
}

DataChunk::Payload DataChunk::GetPayload(const size_t packet_id)
{
    using namespace constants;

    if (packet_id > packets_count_ || packet_id < 1)
        return {};

    const auto& begin = data_.begin() + max_packet_payload_elements * (packet_id - 1);
    size_t payload_size = std::min(size_t(data_.end() - begin), max_packet_payload_elements);

    return {begin, payload_size};
}