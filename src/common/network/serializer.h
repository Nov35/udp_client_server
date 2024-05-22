#pragma once

#include "packet_types.h"

size_t Serialize(const Packet::Ptr packet, BinaryData &data);

size_t Deserialize(BinaryData &data, Packet::Ptr packet);
