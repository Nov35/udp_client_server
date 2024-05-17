#pragma once

#include "constants.h"

#include <memory>
#include <vector>

// TODO Provide better research about maximum size of payload guaranteed to be sent without fragmentation

using BinaryData = std::vector<uint8_t>;

enum class PacketType : uint8_t
{
    InitialRequest,
    InitialResponse,

    RangeSettingMessage,

    PacketCheckRequest,
    PacketCheckResponse,

    PayloadMessage
};

struct Packet
{
    using Ptr = std::shared_ptr<Packet>;

    virtual PacketType GetType() = 0;
};

struct InitialRequest : Packet
{
    using Ptr = std::shared_ptr<InitialRequest>;

    uint8_t major_version_;
    uint8_t minor_version_;
    uint8_t patch_version_;
    
    virtual PacketType GetType() override
    {
        return PacketType::InitialRequest;
    }
};

struct InitialResponse : Packet
{
    using Ptr = std::shared_ptr<InitialResponse>;

    bool is_successful_;
    uint8_t error_length_;
    char error_msg_[constants::max_error_length];

    virtual PacketType GetType() override
    {
        return PacketType::InitialResponse;
    }
};

struct RangeSettingMessage : Packet
{
    double range_constant_;

    virtual PacketType GetType() override
    {
        return PacketType::RangeSettingMessage;
    }
};

struct PacketCheckRequest : Packet
{
    uint8_t packets_sent_;

    virtual PacketType GetType() override
    {
        return PacketType::PacketCheckRequest;
    }
};

struct PacketCheckResponse : Packet
{
    uint8_t packets_missing_;
    uint8_t missing_packets_[constants::max_packets];

    virtual PacketType GetType() override
    {
        return PacketType::PacketCheckResponse;
    }
};

struct PayloadMessage : Packet
{
    uint8_t packet_id_;
    uint8_t count_;
    double elements_[constants::max_payload_size];

    virtual PacketType GetType() override
    {
        return PacketType::PayloadMessage;
    }
};
