#include <gtest/gtest.h>
#include "pricelevel.h"

using namespace orderbook;

TEST(PriceLevelTest, defaultConstruction) {
    PriceLevel p;
    EXPECT_EQ(p.getPrice(), 0);
    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, constructionWithPrice) {
    PriceLevel p(99.5);
    EXPECT_EQ(p.getPrice(), 99.5);
    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, addSingleOrder) {
    PriceLevel p(100.0);
    p.add(1, 10);
    EXPECT_EQ(p.getSize(), 1);
    EXPECT_EQ(p.getQty(), 10);
}

TEST(PriceLevelTest, addMultipleOrders) {
    PriceLevel p(100.0);
    p.add(1, 10);
    p.add(2, 20);
    p.add(3, 30);

    EXPECT_EQ(p.getSize(), 3);
    EXPECT_EQ(p.getQty(), 60);
}

TEST(PriceLevelTest, popFromEmpty) {
    PriceLevel p(100.0);
    EXPECT_FALSE(p.pop());
}

TEST(PriceLevelTest, frontReturnsFrontId) {
    PriceLevel p(100.0);
    p.add(42, 10);
    p.add(43, 20);

    EXPECT_EQ(p.front(), 42);
}

TEST(PriceLevelTest, popReturnsFIFOOrder) {
    PriceLevel p(100.0);
    p.add(42, 10);
    p.add(43, 20);

    EXPECT_EQ(p.front(), 42);
    EXPECT_TRUE(p.pop());

    EXPECT_EQ(p.front(), 43);
    EXPECT_TRUE(p.pop());
}

TEST(PriceLevelTest, popUpdatesSize) {
    PriceLevel p(100.0);
    p.add(1, 10);
    p.add(2, 20);

    EXPECT_EQ(p.getSize(), 2);

    p.pop();
    EXPECT_EQ(p.getSize(), 1);

    p.pop();
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, popAllThenPopReturnsFalse) {
    PriceLevel p(100.0);
    p.add(1, 5);

    EXPECT_TRUE(p.pop());
    EXPECT_FALSE(p.pop());
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, removeIds) {
    PriceLevel p(100.0);
    p.add(1, 10);
    p.add(2, 20);
    p.add(3, 30);

    std::unordered_set<uint32_t> toRemove = {1, 3};
    p.removeIds(toRemove);

    EXPECT_EQ(p.getSize(), 1);
    EXPECT_EQ(p.front(), 2);
}
