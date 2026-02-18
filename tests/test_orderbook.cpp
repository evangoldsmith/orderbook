#include <gtest/gtest.h>
#include "orderbook.h"

using namespace orderbook;

TEST(OrderbookTest, construction) {
    Orderbook ob;
    EXPECT_TRUE(true);
}

TEST(OrderbookTest, addBuyOrder) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 3, 10.0));

    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(OrderbookTest, addSellOrder) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 0);
}

TEST(OrderbookTest, getHighestBid) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 3, 10.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 5, 11.0));
    
    EXPECT_EQ(ob.getBidCount(), 2);
    EXPECT_EQ(ob.getHighestBid(), 11.0);
}

TEST(OrderbookTest, getLowestAsk) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 5, 11.0));
    
    EXPECT_EQ(ob.getAskCount(), 2);
    EXPECT_EQ(ob.getLowestAsk(), 10.0);
}

TEST(OrderbookTest, matchBuyerSeller) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    EXPECT_EQ(ob.getAskCount(), 1);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 1, 11.0));
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskPriceLevel(10.0).peek().qty, 4);
}

TEST(OrderbookTest, matchPartialFill) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    EXPECT_EQ(ob.getAskCount(), 1);

    ASSERT_EQ(BookResponse::PARTIALLY_FULFILLED, ob.insertOrder(Side::BUY, 10, 11.0));
    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(11.0).peek().qty, 5);
}