#pragma once

#include <packet_types.h>

void Serialize(Packet::Ptr packet, BinaryData &data);

void Deserialize(BinaryData &data, Packet::Ptr packet);