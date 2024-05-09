#include <gtest/gtest.h>

#include<connection.h>

TEST(UnitTests, ConnectionLibIsLinked)
{
    EXPECT_EQ(103002, Connection::test());
}