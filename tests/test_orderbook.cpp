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

// ==================== Pro-Rata Matching Tests ====================

// Basic proportional allocation: two asks of equal size should each lose
// half their qty when a buy comes in for the total.
TEST(ProRataTest, equalSizedOrdersSplitEvenly) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 10, 10.0);
    ob.insertOrder(Side::SELL, 10, 10.0);
    EXPECT_EQ(ob.getAskCount(), 2);

    // Buy 10 at 10.0 — each order is 50% of the 20 total, gets 5 filled
    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 2);

    std::deque<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q[0].qty, 5);
    EXPECT_EQ(q[1].qty, 5);
}

// Unequal orders: 40/60 split. Buy 10 from total of 100.
// Order A (40): floor(10 * 40/100) = floor(4.0) = 4
// Order B (60): floor(10 * 60/100) = floor(6.0) = 6
TEST(ProRataTest, unequalOrdersProportionalAllocation) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 40, 10.0);
    ob.insertOrder(Side::SELL, 60, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    std::deque<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q[0].qty, 36);  // 40 - 4
    EXPECT_EQ(q[1].qty, 54);  // 60 - 6
}

// Rounding: three orders of 10 each (total 30). Buy 10.
// Each gets floor(10 * 10/30) = floor(3.33) = 3. That's 9 filled.
// Remainder of 1 goes to the first order (FIFO).
TEST(ProRataTest, remainderDistributedFIFO) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 10, 10.0);
    ob.insertOrder(Side::SELL, 10, 10.0);
    ob.insertOrder(Side::SELL, 10, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    std::deque<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q[0].qty, 6);  // 10 - 3 proportional - 1 remainder
    EXPECT_EQ(q[1].qty, 7);  // 10 - 3 proportional
    EXPECT_EQ(q[2].qty, 7);  // 10 - 3 proportional
}

// Buy consumes entire price level — all orders fully filled and removed.
TEST(ProRataTest, fullFillRemovesAllOrders) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 5, 10.0);
    ob.insertOrder(Side::SELL, 5, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 0);
}

// Buy is larger than entire level — partial fill, remainder added to book.
TEST(ProRataTest, buyLargerThanLevel) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 3, 10.0);
    ob.insertOrder(Side::SELL, 7, 10.0);

    ASSERT_EQ(BookResponse::PARTIALLY_FULFILLED, ob.insertOrder(Side::BUY, 20, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(10.0).peek().qty, 10);
}

// Sell-side pro-rata: incoming sell matched against resting bids.
// Two bids of 20 each (total 40). Sell 10.
// Each gets floor(10 * 20/40) = 5.
TEST(ProRataTest, sellSideProportionalMatch) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::BUY, 20, 10.0);
    ob.insertOrder(Side::BUY, 20, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::SELL, 10, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);

    std::deque<Order>& q = ob.getBidPriceLevel(10.0).getQ();
    EXPECT_EQ(q[0].qty, 15);
    EXPECT_EQ(q[1].qty, 15);
}

// Sell-side with remainder: three bids of 10 (total 30). Sell 7.
// Each gets floor(7 * 10/30) = floor(2.33) = 2. That's 6 filled.
// Remainder of 1 goes to first bid (FIFO).
TEST(ProRataTest, sellSideRemainderFIFO) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::BUY, 10, 10.0);
    ob.insertOrder(Side::BUY, 10, 10.0);
    ob.insertOrder(Side::BUY, 10, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::SELL, 7, 10.0));

    std::deque<Order>& q = ob.getBidPriceLevel(10.0).getQ();
    EXPECT_EQ(q[0].qty, 7);  // 10 - 2 proportional - 1 remainder
    EXPECT_EQ(q[1].qty, 8);  // 10 - 2
    EXPECT_EQ(q[2].qty, 8);  // 10 - 2
}

// Small order gets 0 proportional share but still receives from remainder.
// Orders: 1, 99 (total 100). Buy 5.
// Order A (1):  floor(5 * 1/100) = 0
// Order B (99): floor(5 * 99/100) = floor(4.95) = 4
// Filled = 4, remainder = 1 → Order A gets 1 via FIFO.
TEST(ProRataTest, smallOrderGetsRemainderOnly) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 1, 10.0);
    ob.insertOrder(Side::SELL, 99, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

    std::deque<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q[1].qty, 95);  // 99 - 4

    // After next operation, the zeroed order should get cleaned up
}

// No match when prices don't cross — same as price-time.
TEST(ProRataTest, noMatchWhenPricesDontCross) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 10, 10.0);
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::BUY, 10, 9.0));

    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 1);
}

// Single resting order behaves like price-time — gets the full fill.
TEST(ProRataTest, singleOrderGetsFullFill) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 10, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 7, 10.0));

    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getAskPriceLevel(10.0).peek().qty, 3);
}

// Book works correctly after pro-rata completely clears a level.
TEST(ProRataTest, bookWorksAfterLevelCleared) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 5, 10.0);
    ob.insertOrder(Side::SELL, 5, 10.0);

    ASSERT_EQ(BookResponse::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));
    EXPECT_EQ(ob.getAskCount(), 0);

    // Insert new orders — book should still function
    ASSERT_EQ(BookResponse::PENDING, ob.insertOrder(Side::SELL, 3, 11.0));
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getLowestAsk(), 11.0);
}