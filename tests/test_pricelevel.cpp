#include <gtest/gtest.h>
#include "pricelevel.h"

using namespace orderbook;

TEST(PriceLevelTest, DefaultConstruction) {
    PriceLevel p;
    EXPECT_EQ(p.getPrice(), 0);
    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, ConstructionWithPrice) {
    PriceLevel p(99.5);
    EXPECT_EQ(p.getPrice(), 99.5);
    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, AddSingleOrder) {
    PriceLevel p(100.0);
    Order o(BUY, 10, 100.0);
    EXPECT_TRUE(p.add(o));
    EXPECT_EQ(p.getSize(), 1);
    EXPECT_EQ(p.getQty(), 10);
}

TEST(PriceLevelTest, AddMultipleOrders) {
    PriceLevel p(100.0);
    Order o1(BUY, 10, 100.0);
    Order o2(BUY, 20, 100.0);
    Order o3(BUY, 30, 100.0);

    p.add(o1);
    p.add(o2);
    p.add(o3);

    EXPECT_EQ(p.getSize(), 3);
    EXPECT_EQ(p.getQty(), 60);
}

TEST(PriceLevelTest, PopFromEmpty) {
    PriceLevel p(100.0);
    Order o;
    EXPECT_FALSE(p.pop(o));
}

TEST(PriceLevelTest, PopReturnsFIFOOrder) {
    PriceLevel p(100.0);
    Order o1(BUY, 10, 100.0);
    Order o2(BUY, 20, 100.0);

    uint32_t firstId = o1.id;
    uint32_t secondId = o2.id;

    p.add(o1);
    p.add(o2);

    Order popped;
    EXPECT_TRUE(p.pop(popped));
    EXPECT_EQ(popped.id, firstId);
    EXPECT_EQ(popped.qty, 10);

    EXPECT_TRUE(p.pop(popped));
    EXPECT_EQ(popped.id, secondId);
    EXPECT_EQ(popped.qty, 20);
}

TEST(PriceLevelTest, PopUpdatesQtyAndSize) {
    PriceLevel p(100.0);
    Order o1(BUY, 10, 100.0);
    Order o2(BUY, 20, 100.0);

    p.add(o1);
    p.add(o2);

    EXPECT_EQ(p.getQty(), 30);
    EXPECT_EQ(p.getSize(), 2);

    Order popped;
    p.pop(popped);

    EXPECT_EQ(p.getQty(), 20);
    EXPECT_EQ(p.getSize(), 1);

    p.pop(popped);

    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, PopAllThenPopReturnsfalse) {
    PriceLevel p(100.0);
    Order o(BUY, 5, 100.0);
    p.add(o);

    Order popped;
    EXPECT_TRUE(p.pop(popped));
    EXPECT_FALSE(p.pop(popped));
    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}
