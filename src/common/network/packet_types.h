#pragma once

#include <ser20/types/polymorphic.hpp>

#include <asio/streambuf.hpp>

#include <cstdint>

// TODO Provide better research about maximum size of payload guaranteed to be sent without fragmentation
constexpr uint32_t MAX_PAYLOAD = 128;

using BinaryData = asio::streambuf;

enum class PacketType : uint8_t
{
    InitialRequest,
    InitialResponse,

    RangeSettingMessage,

    PacketCheckRequest,
    PacketCheckResponse,

    PayloadMessage,

//! To be removed
    TEST_TYPE = UINT8_MAX
};

struct Packet
{
    using Ptr = std::shared_ptr<Packet>;

    virtual PacketType GetType() = 0;
};

struct InitialRequest : Packet
{
    uint8_t major_version_;
    uint8_t minor_version_;
    uint8_t patch_version_;
    
    virtual PacketType GetType() override
    {
        return PacketType::InitialRequest;
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(major_version_, minor_version_, patch_version_);
    }
};

struct InitialResponse : Packet
{
    bool is_successful_;
    uint8_t error_length_;
    char error_msg_[UINT8_MAX];

    virtual PacketType GetType() override
    {
        return PacketType::InitialResponse;
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(is_successful_, error_length_, error_msg_);
    }
};

struct RangeSettingMessage : Packet
{
    double range_constant_;

    virtual PacketType GetType() override
    {
        return PacketType::RangeSettingMessage;
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(range_constant_);
    }
};

struct PacketCheckRequest : Packet
{
    uint8_t packets_sent_;

    virtual PacketType GetType() override
    {
        return PacketType::PacketCheckRequest;
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(packets_sent_);
    }
};

struct PacketCheckResponse : Packet
{
    uint8_t packets_missing_;
    uint8_t missing_packets_[UINT8_MAX];

    virtual PacketType GetType() override
    {
        return PacketType::PacketCheckResponse;
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(packets_missing_, missing_packets_);
    }
};

struct PayloadMessage : Packet
{
    uint8_t packet_id_;
    uint8_t count_;
    double elements_[MAX_PAYLOAD];

    virtual PacketType GetType() override
    {
        return PacketType::PayloadMessage;
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(packet_id_, count_, elements_);
    }
};

SER20_REGISTER_TYPE(InitialRequest)
SER20_REGISTER_TYPE(InitialResponse)
SER20_REGISTER_TYPE(RangeSettingMessage)
SER20_REGISTER_TYPE(PacketCheckRequest)
SER20_REGISTER_TYPE(PacketCheckResponse)
SER20_REGISTER_TYPE(PayloadMessage)

SER20_REGISTER_POLYMORPHIC_RELATION(Packet, InitialRequest)
SER20_REGISTER_POLYMORPHIC_RELATION(Packet, InitialResponse)
SER20_REGISTER_POLYMORPHIC_RELATION(Packet, RangeSettingMessage)
SER20_REGISTER_POLYMORPHIC_RELATION(Packet, PacketCheckRequest)
SER20_REGISTER_POLYMORPHIC_RELATION(Packet, PacketCheckResponse)
SER20_REGISTER_POLYMORPHIC_RELATION(Packet, PayloadMessage)


//! To be removed
struct TestPacket : Packet
{
    using Ptr = std::shared_ptr<TestPacket>;

    std::string str_;

    virtual PacketType GetType() override
    {
        return PacketType::TEST_TYPE;
    }

    template <class Archive>
    void serialize(Archive &ar)
    {
        ar(str_);
    }
};

SER20_REGISTER_TYPE(TestPacket)
SER20_REGISTER_POLYMORPHIC_RELATION(Packet, TestPacket)
