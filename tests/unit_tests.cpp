#include <gtest/gtest.h>

#include "serializer.h"
#include "client_context_impl.cpp"

#include <asio/io_context.hpp>
#include <asio/steady_timer.hpp>

TEST(UnitTests, InitialRequestDeserealization)
{
    BinaryData buffer;

    InitialRequest::Ptr pt1 = std::make_shared<InitialRequest>();

    pt1->major_version_ = 1;
    pt1->minor_version_ = 2;
    pt1->patch_version_ = 3;

    size_t written = Serialize(*pt1, buffer);
    size_t buff_size = buffer.size();

    ASSERT_EQ(static_cast<PacketType>(buffer[0]), pt1->GetType());

    InitialRequest::Ptr pt2 = std::make_shared<InitialRequest>();
    Deserialize<InitialRequest>({buffer, written}, *pt2);

    EXPECT_EQ(pt1->major_version_, pt2->major_version_);
    EXPECT_EQ(pt1->minor_version_, pt2->minor_version_);
    EXPECT_EQ(pt1->patch_version_, pt2->patch_version_);
}

TEST(UnitTests, PayloadPacketDeserealization)
{
    BinaryData buffer;

    PayloadMessage::Ptr pt1 = std::make_shared<PayloadMessage>();

    pt1->packet_id_ = 4;
    pt1->payload_size_ = 124;

    for (int i = 1; i <= pt1->payload_size_; ++i)
        pt1->payload_[i - 1] = double(i) / 1000000.0;

    size_t written = Serialize<PayloadMessage>(*pt1, buffer);
    size_t buff_size = buffer.size();

    ASSERT_EQ(static_cast<PacketType>(buffer[0]), pt1->GetType());

    PayloadMessage::Ptr pt2 = std::make_shared<PayloadMessage>();
    Deserialize<PayloadMessage>({buffer, written}, *pt2);

    EXPECT_EQ(pt1->packet_id_, pt2->packet_id_);
    EXPECT_EQ(pt1->payload_size_, pt2->payload_size_);

    for (int i = 0; i < pt1->payload_size_; ++i)
        EXPECT_EQ(pt1->payload_[i], pt2->payload_[i]);
}

TEST(UnitTests, GeneratedDataIsAccessedCorrectly)
{
    asio::io_context io;
    ClientContextImpl context(io);
    context.PrepareData(100.0);

    auto &data = context.GetData();

    std::vector<double> dest(data.size());
    auto dest_ptr = dest.data();

    DataChunk chunk = context.GetChunkOfData();
    while (chunk.GetPacketsCount() != 0)
    {
        for (int i = 1; i <= chunk.GetPacketsCount(); ++i)
        {
            auto [begin, end] = chunk.GetPayload(i);
            dest_ptr = std::copy(begin, end, dest_ptr);
        }
        context.NextIteration();
        chunk = context.GetChunkOfData();
    }

    ASSERT_TRUE(std::equal(data.begin(), data.end(), dest.begin()));
}
