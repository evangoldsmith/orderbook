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

// Sell-side matching tests

TEST(OrderbookTest, matchSellerBuyer) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    EXPECT_EQ(ob.getBidCount(), 1);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::SELL, 1, 9.0));
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidPriceLevel(10.0).peek().qty, 4);
}

TEST(OrderbookTest, matchSellerFullFill) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::SELL, 5, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(OrderbookTest, matchSellerPartialFill) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 3, 10.0));
    ASSERT_EQ(BookResponse::PARTIALLY_FULFILLED, ob.insertOrder(Side::SELL, 7, 9.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getAskPriceLevel(9.0).peek().qty, 4);
}

// Exact price match

TEST(OrderbookTest, exactPriceMatch) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(OrderbookTest, exactPriceMatchSellSide) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::SELL, 5, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

// Multi-level matching

TEST(OrderbookTest, buySweepsMultipleAskLevels) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 4, 11.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 5, 12.0));
    EXPECT_EQ(ob.getAskCount(), 3);

    // Buy 10 at 12.0 should sweep all three levels (3 + 4 + 3 from 12.0 level)
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 10, 12.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getLowestAsk(), 12.0);
    EXPECT_EQ(ob.getAskPriceLevel(12.0).peek().qty, 2);
}

TEST(OrderbookTest, sellSweepsMultipleBidLevels) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 3, 12.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 4, 11.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    EXPECT_EQ(ob.getBidCount(), 3);

    // Sell 10 at 10.0 should sweep highest first: 3@12 + 4@11 + 3@10
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::SELL, 10, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getHighestBid(), 10.0);
    EXPECT_EQ(ob.getBidPriceLevel(10.0).peek().qty, 2);
}

TEST(OrderbookTest, buySweepsAllLevelsAndHasRemainder) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 2, 11.0));

    // Buy 10 at 11.0, only 5 available — remainder goes to book
    ASSERT_EQ(BookResponse::PARTIALLY_FULFILLED, ob.insertOrder(Side::BUY, 10, 11.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(11.0).peek().qty, 5);
}

// Edge cases

TEST(OrderbookTest, noMatchWhenPricesDontCross) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 5, 9.0));

    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 1);
}

TEST(OrderbookTest, matchClearsEntireBook) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 0);

    // Book should still work after being emptied
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 3, 9.0));
    EXPECT_EQ(ob.getBidCount(), 1);
}

TEST(OrderbookTest, multipleOrdersSamePriceLevel) {
    Orderbook ob;

    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 4, 10.0));
    EXPECT_EQ(ob.getAskCount(), 2);

    // Buy 5 should consume first order (3) and partially fill second (2 of 4)
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getAskPriceLevel(10.0).peek().qty, 2);
}