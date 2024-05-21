#include "serializer.h"

#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/ext/inheritance.h>
#include <bitsery/ext/pointer.h>
#include <bitsery/ext/std_smart_ptr.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>

#include <cassert>
#include <memory>

using bitsery::ext::BaseClass;
using bitsery::ext::PointerObserver;
using bitsery::ext::StdSmartPtr;

//TODO Add endianness handling to support differebnt architectures

template <typename S>
void serialize(S &s, Packet &o)
{
}

template <typename S>
void serialize(S &s, InitialRequest &o)
{
    s.value1b(o.major_version_);
    s.value1b(o.minor_version_);
    s.value1b(o.patch_version_);
}

template <typename S>
void serialize(S &s, ServerResponse &o)
{
    s.value1b(o.is_successful_);
    s.text1b(o.message_, constants::max_error_length);
}

template <typename S>
void serialize(S &s, RangeSettingMessage &o)
{
    s.value8b(o.range_constant_);
}

template <typename S>
void serialize(S &s, PacketCheckRequest &o)
{
    s.value1b(o.packets_sent_);
}

template <typename S>
void serialize(S &s, PacketCheckResponse &o)
{
    s.value1b(o.packets_missing_);
    s.container1b(o.missing_packets_);
}

template <typename S>
void serialize(S &s, PayloadMessage &o)
{
    s.value1b(o.packet_id_);
    s.value1b(o.payload_size_);
    s.container8b(o.payload_);
}

namespace bitsery
{
    namespace ext
    {

        template <>
        struct PolymorphicBaseClass<Packet>
            : PolymorphicDerivedClasses<InitialRequest,
                                        ServerResponse,
                                        RangeSettingMessage,
                                        PacketCheckRequest,
                                        PacketCheckResponse,
                                        PayloadMessage>
        {
        };

    }
}

using PacketHierarchy = bitsery::ext::PolymorphicClassesList<Packet>;

using Writer = bitsery::OutputBufferAdapter<BinaryData>;
using Reader = bitsery::InputBufferAdapter<BinaryData>;

using TContext =
    std::tuple<bitsery::ext::PointerLinkingContext,
               bitsery::ext::PolymorphicContext<bitsery::ext::StandardRTTI>>;

using PacketSerializer = bitsery::Serializer<Writer, TContext>;
using PacketDeserializer = bitsery::Deserializer<Reader, TContext>;

struct PacketHolder
{
    PacketHolder(Packet::Ptr ptr) : type_(ptr->GetType()), ptr_(ptr)
    {
    }

    PacketType type_;
    Packet::Ptr ptr_;

    template <typename S>
    void serialize(S &s)
    {
        s.value1b(type_);
        s.ext(ptr_, StdSmartPtr{});
    }
};

size_t Serialize(const Packet::Ptr packet, BinaryData &buffer)
{
    TContext ctx{};
    std::get<1>(ctx).registerBasesList<PacketSerializer>(PacketHierarchy{});

    PacketSerializer ser{ctx, buffer};
    PacketHolder holder(std::move(packet));
    ser.object(holder);
    ser.adapter().flush();

    return ser.adapter().writtenBytesCount();
}

size_t Deserialize(const BinaryData &buffer, Packet::Ptr packet)
{
    TContext ctx{};
    std::get<1>(ctx).registerBasesList<PacketDeserializer>(PacketHierarchy{});

    size_t bytes_read;
    PacketDeserializer des{ctx, buffer.begin(), bytes_read};
    PacketHolder holder(std::move(packet));
    des.object(holder);
    std::get<0>(ctx).clearSharedState();

    return bytes_read;
}
