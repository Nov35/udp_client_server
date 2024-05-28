#pragma once

#include "constants.h"

#include <bit>
#include <memory>
#include <vector>
#include <string>

using BinaryData = std::vector<uint8_t>;
using Payload = double[constants::max_payload_elements];

//TODO Try approach without dynamic allocation/polymorphysm

enum class PacketType : uint8_t
{
    InitialRequest,
    ServerResponse,

    RangeSettingMessage,

    PacketCheckRequest,
    PacketCheckResponse,

    PayloadMessage
};

struct Packet
{
    PacketType type_;
    uint32_t chunk_;
};

struct InitialRequest : Packet
{
    using Ptr = std::shared_ptr<InitialRequest>;

    uint8_t major_version_;
    uint8_t minor_version_;
    uint8_t patch_version_;

    uint8_t endianness_;

    PacketType GetType() const
    {
        return PacketType::InitialRequest;
    }
};

struct ServerResponse : Packet
{
    using Ptr = std::shared_ptr<ServerResponse>;

    bool is_successful_;
    uint16_t msg_lenght_;
    char message_[constants::max_msg_length];

    PacketType GetType() const
    {
        return PacketType::ServerResponse;
    }
};

struct RangeSettingMessage : Packet
{
    using Ptr = std::shared_ptr<RangeSettingMessage>;

    double range_constant_;

    PacketType GetType() const
    {
        return PacketType::RangeSettingMessage;
    }
};

struct PacketCheckRequest : Packet
{
    using Ptr = std::shared_ptr<PacketCheckRequest>;

    uint8_t packets_sent_;

    PacketType GetType() const
    {
        return PacketType::PacketCheckRequest;
    }
};

struct PacketCheckResponse : Packet
{
    using Ptr = std::shared_ptr<PacketCheckResponse>;

    uint8_t packets_missing_;
    uint8_t missing_packets_[constants::packets_chunk_size];

    PacketType GetType() const
    {
        return PacketType::PacketCheckResponse;
    }
};

struct PayloadMessage : Packet
{
    using Ptr = std::shared_ptr<PayloadMessage>;

    uint8_t packet_id_;
    uint8_t payload_size_;
    Payload payload_;

    PacketType GetType() const
    {
        return PacketType::PayloadMessage;
    }
};
