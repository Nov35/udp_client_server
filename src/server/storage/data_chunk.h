#pragma once

#include <cstddef>

class DataChunk
{
public:
    struct Payload
    {
        const double *begin_;
        const double *end_;
    };

public:
    DataChunk(const double *begin, const double *end, const size_t count);
    const size_t GetPacketsCount();
    Payload GetPayload(const size_t packet_num);

private:
    const double *begin_;
    const double *end_;
    size_t packets_count_;
};