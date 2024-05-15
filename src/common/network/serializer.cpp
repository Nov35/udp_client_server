#include "serializer.h"
#include<ser20/archives/portable_binary.hpp>

void Serialize(Packet::Ptr packet, BinaryData &data)
{
    std::ostream output(&data);
    
    output << static_cast<uint8_t>(packet->GetType());
    ser20::PortableBinaryOutputArchive oarchive(output);
    oarchive(packet);
}

void Deserialize(BinaryData &data, Packet::Ptr packet)
{
    std::istream input(&data);
    ser20::PortableBinaryInputArchive iarchive(input);
    iarchive(packet);
}