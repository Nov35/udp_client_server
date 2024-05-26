#pragma once

#include "packet_types.h"
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/ext/inheritance.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>

#include <cassert>
#include <memory>

using ReceivedData = std::pair<const BinaryData &, const size_t>;



// TODO Add endianness handling to support different architectures

using bitsery::ext::BaseClass;

template <typename S>
void serialize(S &s, Packet &o)
{
    s.value1b(o.type_);
    s.value4b(o.chunk_);
}

template <typename S>
void serialize(S &s, InitialRequest &o)
{
    s.ext(o, BaseClass<Packet>{});
    s.value1b(o.major_version_);
    s.value1b(o.minor_version_);
    s.value1b(o.patch_version_);
    s.value1b(o.endianness_);
}

template <typename S>
void serialize(S &s, ServerResponse &o)
{
    s.ext(o, BaseClass<Packet>{});
    s.value1b(o.is_successful_);
    s.value2b(o.msg_lenght_);
    s.text1b(o.message_);
}

template <typename S>
void serialize(S &s, RangeSettingMessage &o)
{
    s.ext(o, BaseClass<Packet>{});
    s.value8b(o.range_constant_);
}

template <typename S>
void serialize(S &s, PacketCheckRequest &o)
{
    s.ext(o, BaseClass<Packet>{});
    s.value1b(o.packets_sent_);
}

template <typename S>
void serialize(S &s, PacketCheckResponse &o)
{
    s.ext(o, BaseClass<Packet>{});
    s.value1b(o.packets_missing_);
    s.container1b(o.missing_packets_);
}

template <typename S>
void serialize(S &s, PayloadMessage &o)
{
    s.ext(o, BaseClass<Packet>{});
    s.value1b(o.packet_id_);
    s.value1b(o.payload_size_);
    s.container8b(o.payload_);
}

using Writer = bitsery::OutputBufferAdapter<BinaryData>;
using Reader = bitsery::InputBufferAdapter<BinaryData>;

template <class PacketType>
size_t Serialize(PacketType& packet, BinaryData &buffer)
{
    packet.type_ = packet.GetType();
    return bitsery::quickSerialization<Writer>(buffer, packet);
}

template <class PacketType>
bool Deserialize(const ReceivedData data, PacketType& packet)
{
    const auto&[buffer, size] = data;
    auto[error, result] = bitsery::quickDeserialization<Reader>({buffer.begin(), size}, packet);
    return result;
}
