#pragma once

#include <cstddef>
#include <span>

class DataChunk
{
public:
    using Payload = std::span<const double>;

public:
    DataChunk(Payload data);
    const size_t GetPacketsCount();
    Payload GetPayload(const size_t packet_id);

private:
    Payload data_;
    size_t packets_count_;
};
