#include <gtest/gtest.h>

#include "serializer.h"
#include "client_context_impl.cpp"

#include <asio/io_context.hpp>

TEST(UnitTests, InitialRequestPolymorphicDeserealization)
{
    BinaryData buffer;

    Packet::Ptr base_pt1 = std::make_shared<InitialRequest>();
    auto child_pt1 = static_pointer_cast<InitialRequest>(base_pt1);

    child_pt1->major_version_ = 1;
    child_pt1->minor_version_ = 2;
    child_pt1->patch_version_ = 3;

    size_t written = Serialize(base_pt1, buffer);
    size_t buff_size = buffer.size();

    ASSERT_EQ(static_cast<PacketType>(buffer[0]), base_pt1->GetType());

    Packet::Ptr base_pt2 = std::make_shared<InitialRequest>();
    Deserialize(buffer, base_pt2);

    auto child_pt2 = static_pointer_cast<InitialRequest>(base_pt1);

    EXPECT_EQ(child_pt1->major_version_, child_pt2->major_version_);
    EXPECT_EQ(child_pt1->minor_version_, child_pt2->minor_version_);
    EXPECT_EQ(child_pt1->patch_version_, child_pt2->patch_version_);
}

TEST(UnitTests, PayloadPacketPolymorphicDeserealization)
{
    BinaryData buffer;

    Packet::Ptr base_pt1 = std::make_shared<PayloadMessage>();
    auto child_pt1 = static_pointer_cast<PayloadMessage>(base_pt1);

    child_pt1->packet_id_ = 4;
    child_pt1->payload_size_ = 124;

    for(int i = 1; i <= child_pt1->payload_size_; ++i)
        child_pt1->payload_[i - 1] = double(i) / 1000000.0;

    size_t written = Serialize(base_pt1, buffer);
    size_t buff_size = buffer.size();

    ASSERT_EQ(static_cast<PacketType>(buffer[0]), base_pt1->GetType());

    Packet::Ptr base_pt2 = std::make_shared<PayloadMessage>();
    Deserialize(buffer, base_pt2);

    auto child_pt2 = static_pointer_cast<PayloadMessage>(base_pt1);

    EXPECT_EQ(child_pt1->packet_id_, child_pt2->packet_id_);
    EXPECT_EQ(child_pt1->payload_size_, child_pt2->payload_size_);

    for(int i = 0; i < child_pt1->payload_size_; ++i)
        EXPECT_EQ(child_pt1->payload_[i], child_pt2->payload_[i]);
}

// TEST(UnitTests, GeneratedDataIsAccessedCorrectly)
// {
//     asio::io_context io;
//     LockedContext context(io);
//     context.PrepareData(100.0);

//     auto& data = context.GetData();

//     std::vector<double> dest(data.size());
//     auto dest_ptr = dest.data();

//     DataChunk chunk = context.GetChunkOfData();
//     while(chunk.GetPacketsCount() != 0)
//     {
//         for(int i = 1; i <= chunk.GetPacketsCount(); ++i)
//         {
//             auto[begin, end] = chunk.GetPayload(i);
//             dest_ptr = std::copy(begin, end, dest_ptr);
//         }
//         context.NextChunkOfData();
//         chunk = context.GetChunkOfData();
//     }

//     ASSERT_TRUE(std::equal(data.begin(), data.end(), dest.begin()));
// }
