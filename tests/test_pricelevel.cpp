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
    Order o(Side::BUY, 10, 100.0);
    EXPECT_TRUE(p.add(o));
    EXPECT_EQ(p.getSize(), 1);
    EXPECT_EQ(p.getQty(), 10);
}

TEST(PriceLevelTest, addMultipleOrders) {
    PriceLevel p(100.0);
    Order o1(Side::BUY, 10, 100.0);
    Order o2(Side::BUY, 20, 100.0);
    Order o3(Side::BUY, 30, 100.0);

    p.add(o1);
    p.add(o2);
    p.add(o3);

    EXPECT_EQ(p.getSize(), 3);
    EXPECT_EQ(p.getQty(), 60);
}

TEST(PriceLevelTest, popFromEmpty) {
    PriceLevel p(100.0);
    EXPECT_FALSE(p.pop());
}

TEST(PriceLevelTest, peekReturnsFrontOrder) {
    PriceLevel p(100.0);
    Order o1(Side::BUY, 10, 100.0);
    Order o2(Side::BUY, 20, 100.0);

    uint32_t firstId = o1.id;

    p.add(o1);
    p.add(o2);

    const Order& front = p.peek();
    EXPECT_EQ(front.id, firstId);
    EXPECT_EQ(front.qty, 10);
}

TEST(PriceLevelTest, popReturnsFIFOOrder) {
    PriceLevel p(100.0);
    Order o1(Side::BUY, 10, 100.0);
    Order o2(Side::BUY, 20, 100.0);

    uint32_t firstId = o1.id;
    uint32_t secondId = o2.id;

    p.add(o1);
    p.add(o2);

    EXPECT_EQ(p.peek().id, firstId);
    EXPECT_EQ(p.peek().qty, 10);
    EXPECT_TRUE(p.pop());

    EXPECT_EQ(p.peek().id, secondId);
    EXPECT_EQ(p.peek().qty, 20);
    EXPECT_TRUE(p.pop());
}

TEST(PriceLevelTest, popUpdatesQtyAndSize) {
    PriceLevel p(100.0);
    Order o1(Side::BUY, 10, 100.0);
    Order o2(Side::BUY, 20, 100.0);

    p.add(o1);
    p.add(o2);

    EXPECT_EQ(p.getQty(), 30);
    EXPECT_EQ(p.getSize(), 2);

    p.pop();

    EXPECT_EQ(p.getQty(), 20);
    EXPECT_EQ(p.getSize(), 1);

    p.pop();

    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}

TEST(PriceLevelTest, popAllThenPopReturnsfalse) {
    PriceLevel p(100.0);
    Order o(Side::BUY, 5, 100.0);
    p.add(o);

    EXPECT_TRUE(p.pop());
    EXPECT_FALSE(p.pop());
    EXPECT_EQ(p.getQty(), 0);
    EXPECT_EQ(p.getSize(), 0);
}
