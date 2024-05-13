#include "serializer.h"
#include<cereal/archives/portable_binary.hpp>

void Serialize(Packet::Ptr packet, BinaryData &data)
{
    std::ostream output(&data);
    
    output << static_cast<uint8_t>(packet->GetType());
    cereal::PortableBinaryOutputArchive oarchive(output);
    oarchive(packet);
}

void Deserialize(BinaryData &data, Packet::Ptr packet)
{
    std::istream input(&data);
    cereal::PortableBinaryInputArchive iarchive(input);
    iarchive(packet);
}