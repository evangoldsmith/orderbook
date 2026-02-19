#include <gtest/gtest.h>
#include "booktypes.h"

using namespace orderbook;

TEST(OrderTest, construction) {
    Order b(Side::BUY, 0, 0.0);
    Order s(Side::SELL, 0, 0.0);
    EXPECT_TRUE(true);
}

TEST(OrderTest, uniqueIds) {
    Order o1(Side::BUY, 0, 0.0);
    Order o2(Side::SELL, 0, 0.0);
    Order o3(Side::BUY, 0, 0.0);

    EXPECT_EQ(o1.id, 1);
    EXPECT_EQ(o2.id, 2);
    EXPECT_EQ(o3.id, 3);
}

TEST(OrderTest, timestampIsPopulated) {
    Timestamp before = now();
    Order o(Side::BUY, 10, 100.0);
    Timestamp after = now();

    EXPECT_GT(o.time, 0);
    EXPECT_GE(o.time, before);
    EXPECT_LE(o.time, after);
}
