#pragma once

#include <cstdint>
#include <cstddef>

namespace constants
{
    constexpr size_t max_packet_size = 65'535;
    constexpr size_t reserved = 60;
    constexpr size_t max_safe_size = 1500 - reserved;
    
    constexpr size_t max_packet_payload_elements = max_safe_size / sizeof(double);
    constexpr size_t max_msg_length = max_safe_size < UINT16_MAX ? max_safe_size : UINT16_MAX;

    constexpr size_t packets_chunk_size = UINT8_MAX;
    constexpr size_t elements_in_one_chunk = packets_chunk_size * max_packet_payload_elements;

    constexpr size_t values_to_transfer = 1'000'000;

    constexpr size_t send_attempts = 5;
    constexpr size_t resend_delay_ms = 50;
}