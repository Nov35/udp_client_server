#pragma once

#include <cstdint>
#include <cstddef>

namespace constants
{
    constexpr size_t expected_packet_size = 1500;
    constexpr uint32_t max_payload_size = 128;
    constexpr size_t max_error_length = UINT8_MAX;
    constexpr size_t max_packets = UINT8_MAX;
}