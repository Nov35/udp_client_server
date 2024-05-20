#pragma once

#include <cstdint>
#include <cstddef>

namespace constants
{
    constexpr size_t max_packet_size = 65'535;
    constexpr size_t udp_plus_ip_header = 28;
    constexpr size_t payload_service_data = 3;
    constexpr size_t max_safe_size = 1500 - udp_plus_ip_header - payload_service_data;
    constexpr size_t max_payload_elements = max_safe_size / sizeof(double);

    constexpr size_t max_error_length = UINT8_MAX;
    constexpr size_t packets_chunk_size = UINT8_MAX;
    constexpr size_t iteration_size = packets_chunk_size * max_payload_elements;

    constexpr size_t values_to_transfer = 1'000'000;
}