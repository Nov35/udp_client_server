#pragma once

#include "constants.h"

#include <memory>
#include <vector>

using BinaryData = std::vector<uint8_t>;

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
    using Ptr = std::shared_ptr<Packet>;

    virtual PacketType GetType() = 0;
    virtual ~Packet() = default;
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
    ~InitialRequest() = default;
};

struct ServerResponse : Packet
{
    using Ptr = std::shared_ptr<ServerResponse>;

    bool is_successful_;
    std::string message_;

    virtual PacketType GetType() override
    {
        return PacketType::ServerResponse;
    }
    ~ServerResponse() = default;
};

struct RangeSettingMessage : Packet
{
    using Ptr = std::shared_ptr<RangeSettingMessage>;

    double range_constant_;

    virtual PacketType GetType() override
    {
        return PacketType::RangeSettingMessage;
    }
    ~RangeSettingMessage() = default;
};

struct PacketCheckRequest : Packet
{
    using Ptr = std::shared_ptr<PacketCheckRequest>;

    uint8_t packets_sent_;

    virtual PacketType GetType() override
    {
        return PacketType::PacketCheckRequest;
    }
    ~PacketCheckRequest() = default;
};

struct PacketCheckResponse : Packet
{
    using Ptr = std::shared_ptr<PacketCheckResponse>;

    uint8_t packets_missing_;
    uint8_t missing_packets_[constants::packets_chunk_size];

    virtual PacketType GetType() override
    {
        return PacketType::PacketCheckResponse;
    }
    ~PacketCheckResponse() = default;
};

struct PayloadMessage : Packet
{
    using Ptr = std::shared_ptr<PayloadMessage>;

    uint8_t packet_id_;
    uint8_t payload_size_;
    double payload_[constants::max_payload_elements];

    virtual PacketType GetType() override
    {
        return PacketType::PayloadMessage;
    }

    ~PayloadMessage() = default;
};
