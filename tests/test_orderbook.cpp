#include <gtest/gtest.h>
#include <unordered_set>
#include "orderbook.h"

using namespace orderbook;

TEST(OrderbookTest, construction) {
    Orderbook ob;
    EXPECT_TRUE(true);
}

TEST(OrderbookTest, addBuyOrder) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 3, 10.0));

    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(OrderbookTest, addSellOrder) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 0);
}

TEST(OrderbookTest, getHighestBid) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 3, 10.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 5, 11.0));
    
    EXPECT_EQ(ob.getBidCount(), 2);
    EXPECT_EQ(ob.getHighestBid(), 11.0);
}

TEST(OrderbookTest, getLowestAsk) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 5, 11.0));
    
    EXPECT_EQ(ob.getAskCount(), 2);
    EXPECT_EQ(ob.getLowestAsk(), 10.0);
}

TEST(OrderbookTest, matchBuyerSeller) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    EXPECT_EQ(ob.getAskCount(), 1);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 1, 11.0));
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskPriceLevel(10.0).peek().qty, 4);
}

TEST(OrderbookTest, matchPartialFill) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    EXPECT_EQ(ob.getAskCount(), 1);

    ASSERT_EQ(Status::PARTIALLY_FULFILLED, ob.insertOrder(Side::BUY, 10, 11.0));
    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(11.0).peek().qty, 5);
}

// Sell-side matching tests

TEST(OrderbookTest, matchSellerBuyer) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    EXPECT_EQ(ob.getBidCount(), 1);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::SELL, 1, 9.0));
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidPriceLevel(10.0).peek().qty, 4);
}

TEST(OrderbookTest, matchSellerFullFill) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::SELL, 5, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(OrderbookTest, matchSellerPartialFill) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 3, 10.0));
    ASSERT_EQ(Status::PARTIALLY_FULFILLED, ob.insertOrder(Side::SELL, 7, 9.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getAskPriceLevel(9.0).peek().qty, 4);
}

// Exact price match

TEST(OrderbookTest, exactPriceMatch) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(OrderbookTest, exactPriceMatchSellSide) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::SELL, 5, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

// Multi-level matching

TEST(OrderbookTest, buySweepsMultipleAskLevels) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 4, 11.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 5, 12.0));
    EXPECT_EQ(ob.getAskCount(), 3);

    // Buy 10 at 12.0 should sweep all three levels (3 + 4 + 3 from 12.0 level)
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 10, 12.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getLowestAsk(), 12.0);
    EXPECT_EQ(ob.getAskPriceLevel(12.0).peek().qty, 2);
}

TEST(OrderbookTest, sellSweepsMultipleBidLevels) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 3, 12.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 4, 11.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    EXPECT_EQ(ob.getBidCount(), 3);

    // Sell 10 at 10.0 should sweep highest first: 3@12 + 4@11 + 3@10
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::SELL, 10, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getHighestBid(), 10.0);
    EXPECT_EQ(ob.getBidPriceLevel(10.0).peek().qty, 2);
}

TEST(OrderbookTest, buySweepsAllLevelsAndHasRemainder) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 2, 11.0));

    // Buy 10 at 11.0, only 5 available — remainder goes to book
    ASSERT_EQ(Status::PARTIALLY_FULFILLED, ob.insertOrder(Side::BUY, 10, 11.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(11.0).peek().qty, 5);
}

// Edge cases

TEST(OrderbookTest, noMatchWhenPricesDontCross) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 5, 9.0));

    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 1);
}

TEST(OrderbookTest, matchClearsEntireBook) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 5, 10.0));
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 0);

    // Book should still work after being emptied
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 3, 9.0));
    EXPECT_EQ(ob.getBidCount(), 1);
}

TEST(OrderbookTest, multipleOrdersSamePriceLevel) {
    Orderbook ob;

    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 3, 10.0));
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 4, 10.0));
    EXPECT_EQ(ob.getAskCount(), 2);

    // Buy 5 should consume first order (3) and partially fill second (2 of 4)
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

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
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 2);

    std::list<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q.front().qty, 5);
    EXPECT_EQ(std::next(q.begin())->qty, 5);
}

// Unequal orders: 40/60 split. Buy 10 from total of 100.
// Order A (40): floor(10 * 40/100) = floor(4.0) = 4
// Order B (60): floor(10 * 60/100) = floor(6.0) = 6
TEST(ProRataTest, unequalOrdersProportionalAllocation) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 40, 10.0);
    ob.insertOrder(Side::SELL, 60, 10.0);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    std::list<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q.front().qty, 36);  // 40 - 4
    EXPECT_EQ(std::next(q.begin())->qty, 54);  // 60 - 6
}

// Rounding: three orders of 10 each (total 30). Buy 10.
// Each gets floor(10 * 10/30) = floor(3.33) = 3. That's 9 filled.
// Remainder of 1 goes to the first order (FIFO).
TEST(ProRataTest, remainderDistributedFIFO) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 10, 10.0);
    ob.insertOrder(Side::SELL, 10, 10.0);
    ob.insertOrder(Side::SELL, 10, 10.0);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    std::list<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q.front().qty, 6);  // 10 - 3 proportional - 1 remainder
    EXPECT_EQ(std::next(q.begin())->qty, 7);  // 10 - 3 proportional
    EXPECT_EQ(std::next(q.begin(), 2)->qty, 7);  // 10 - 3 proportional
}

// Buy consumes entire price level — all orders fully filled and removed.
TEST(ProRataTest, fullFillRemovesAllOrders) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 5, 10.0);
    ob.insertOrder(Side::SELL, 5, 10.0);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 0);
}

// Buy is larger than entire level — partial fill, remainder added to book.
TEST(ProRataTest, buyLargerThanLevel) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 3, 10.0);
    ob.insertOrder(Side::SELL, 7, 10.0);

    ASSERT_EQ(Status::PARTIALLY_FULFILLED, ob.insertOrder(Side::BUY, 20, 10.0));

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

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::SELL, 10, 10.0));

    EXPECT_EQ(ob.getAskCount(), 0);

    std::list<Order>& q = ob.getBidPriceLevel(10.0).getQ();
    EXPECT_EQ(q.front().qty, 15);
    EXPECT_EQ(std::next(q.begin())->qty, 15);
}

// Sell-side with remainder: three bids of 10 (total 30). Sell 7.
// Each gets floor(7 * 10/30) = floor(2.33) = 2. That's 6 filled.
// Remainder of 1 goes to first bid (FIFO).
TEST(ProRataTest, sellSideRemainderFIFO) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::BUY, 10, 10.0);
    ob.insertOrder(Side::BUY, 10, 10.0);
    ob.insertOrder(Side::BUY, 10, 10.0);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::SELL, 7, 10.0));

    std::list<Order>& q = ob.getBidPriceLevel(10.0).getQ();
    EXPECT_EQ(q.front().qty, 7);  // 10 - 2 proportional - 1 remainder
    EXPECT_EQ(std::next(q.begin())->qty, 8);  // 10 - 2
    EXPECT_EQ(std::next(q.begin(), 2)->qty, 8);  // 10 - 2
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

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 5, 10.0));

    std::list<Order>& q = ob.getAskPriceLevel(10.0).getQ();
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.front().qty, 95);  // 99 - 4

    // After next operation, the zeroed order should get cleaned up
}

// No match when prices don't cross — same as price-time.
TEST(ProRataTest, noMatchWhenPricesDontCross) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 10, 10.0);
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 10, 9.0));

    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 1);
}

// Single resting order behaves like price-time — gets the full fill.
TEST(ProRataTest, singleOrderGetsFullFill) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 10, 10.0);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 7, 10.0));

    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getAskPriceLevel(10.0).peek().qty, 3);
}

// Book works correctly after pro-rata completely clears a level.
TEST(ProRataTest, bookWorksAfterLevelCleared) {
    Orderbook ob(MatchingMode::PRO_RATA);

    ob.insertOrder(Side::SELL, 5, 10.0);
    ob.insertOrder(Side::SELL, 5, 10.0);

    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(Side::BUY, 10, 10.0));
    EXPECT_EQ(ob.getAskCount(), 0);

    // Insert new orders — book should still function
    ASSERT_EQ(Status::PENDING, ob.insertOrder(Side::SELL, 3, 11.0));
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getLowestAsk(), 11.0);
}

// ==================== cancelOrder Tests ====================

TEST(CancelOrderTest, cancelBuyReturnsTrueAndReducesCount) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 10, 50.0);

    uint32_t id = ob.getBidPriceLevel(50.0).peek().id;
    EXPECT_TRUE(ob.cancelOrder(id));
    EXPECT_EQ(ob.getBidCount(), 0);
}

TEST(CancelOrderTest, cancelSellReturnsTrueAndReducesCount) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 10, 50.0);

    uint32_t id = ob.getAskPriceLevel(50.0).peek().id;
    EXPECT_TRUE(ob.cancelOrder(id));
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(CancelOrderTest, cancelInvalidIdReturnsFalse) {
    Orderbook ob;
    EXPECT_FALSE(ob.cancelOrder(9999));
}

TEST(CancelOrderTest, cancelledOrderNotAccessibleViaGetOrder) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 5, 20.0);

    uint32_t id = ob.getBidPriceLevel(20.0).peek().id;
    ob.cancelOrder(id);
    EXPECT_THROW(ob.getOrder(id), std::out_of_range);
}

TEST(CancelOrderTest, cancelMiddleOrderPreservesFIFO) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 5, 10.0);
    ob.insertOrder(Side::BUY, 10, 10.0);
    ob.insertOrder(Side::BUY, 7, 10.0);

    std::list<Order>& q = ob.getBidPriceLevel(10.0).getQ();
    uint32_t middleId = std::next(q.begin())->id;

    EXPECT_TRUE(ob.cancelOrder(middleId));

    EXPECT_EQ(ob.getBidCount(), 2);
    EXPECT_EQ(ob.getBidPriceLevel(10.0).peek().qty, 5);          // first still intact
    EXPECT_EQ(std::next(q.begin())->qty, 7);                     // last slides up
}

TEST(CancelOrderTest, cancelledOrderDoesNotParticipateInMatch) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 5, 10.0);

    uint32_t id = ob.getAskPriceLevel(10.0).peek().id;
    EXPECT_TRUE(ob.cancelOrder(id));

    // No resting sell remains, buy should go to book as PENDING
    EXPECT_EQ(Status::PENDING, ob.insertOrder(Side::BUY, 5, 10.0));
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(CancelOrderTest, cancelUpdatesQty) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 10, 50.0);
    ob.insertOrder(Side::BUY, 20, 50.0);

    uint32_t id = ob.getBidPriceLevel(50.0).peek().id;
    ob.cancelOrder(id);

    EXPECT_EQ(ob.getBidPriceLevel(50.0).getQty(), 20);
}

// ==================== getOrder Tests ====================

TEST(GetOrderTest, retrieveBuyOrder) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 10, 50.0);

    uint32_t id = ob.getBidPriceLevel(50.0).peek().id;
    const Order& o = ob.getOrder(id);

    EXPECT_EQ(o.id, id);
    EXPECT_EQ(o.side, Side::BUY);
    EXPECT_EQ(o.price, 50.0);
    EXPECT_EQ(o.qty, 10);
}

TEST(GetOrderTest, retrieveSellOrder) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 7, 99.0);

    uint32_t id = ob.getAskPriceLevel(99.0).peek().id;
    const Order& o = ob.getOrder(id);

    EXPECT_EQ(o.id, id);
    EXPECT_EQ(o.side, Side::SELL);
    EXPECT_EQ(o.price, 99.0);
    EXPECT_EQ(o.qty, 7);
}

TEST(GetOrderTest, retrieveMultipleOrders) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 5, 10.0);
    ob.insertOrder(Side::BUY, 8, 11.0);

    uint32_t id1 = ob.getBidPriceLevel(10.0).peek().id;
    uint32_t id2 = ob.getBidPriceLevel(11.0).peek().id;

    EXPECT_EQ(ob.getOrder(id1).qty, 5);
    EXPECT_EQ(ob.getOrder(id1).price, 10.0);
    EXPECT_EQ(ob.getOrder(id2).qty, 8);
    EXPECT_EQ(ob.getOrder(id2).price, 11.0);
}

TEST(GetOrderTest, qtyReflectsPartialFill) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 10, 50.0);

    uint32_t id = ob.getAskPriceLevel(50.0).peek().id;

    // Partially fill the resting sell order
    ob.insertOrder(Side::BUY, 4, 50.0);

    EXPECT_EQ(ob.getOrder(id).qty, 6);
}

TEST(GetOrderTest, invalidIdThrows) {
    Orderbook ob;
    EXPECT_THROW(ob.getOrder(9999), std::out_of_range);
}

// ==================== correctOrder Tests ====================

TEST(CorrectOrderTest, nonExistentIdReturnsError) {
    Orderbook ob;
    Order updated(Side::BUY, 5, 10.0);
    EXPECT_EQ(Status::ERROR, ob.correctOrder(9999, updated));
}

TEST(CorrectOrderTest, correctQtyStaysPending) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);

    Order updated(Side::BUY, 5, 50.0);
    EXPECT_EQ(Status::PENDING, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(50.0).peek().qty, 5);
}

TEST(CorrectOrderTest, correctPriceMovesPriceLevel) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);

    Order updated(Side::BUY, 10, 60.0);
    EXPECT_EQ(Status::PENDING, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getHighestBid(), 60.0);
    EXPECT_EQ(ob.getBidPriceLevel(60.0).peek().qty, 10);
}

TEST(CorrectOrderTest, correctPriceRemovesFromOldLevel) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);

    Order updated(Side::BUY, 10, 60.0);
    ob.correctOrder(id, updated);

    // Only one bid and it's at the new price — old level is gone
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getHighestBid(), 60.0);
}

TEST(CorrectOrderTest, correctOrderPreservesId) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);

    Order updated(Side::BUY, 7, 50.0);
    ob.correctOrder(id, updated);

    const Order& o = ob.getOrder(id);
    EXPECT_EQ(o.id, id);
    EXPECT_EQ(o.qty, 7);
    EXPECT_EQ(o.price, 50.0);
}

TEST(CorrectOrderTest, correctSideBuyToSell) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);

    Order updated(Side::SELL, 10, 60.0);
    EXPECT_EQ(Status::PENDING, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getLowestAsk(), 60.0);
}

TEST(CorrectOrderTest, correctSideSellToBuy) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::SELL, 10, 60.0);

    Order updated(Side::BUY, 10, 50.0);
    EXPECT_EQ(Status::PENDING, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getHighestBid(), 50.0);
}

// Correct a buy to a price that crosses a resting sell → immediate fill
TEST(CorrectOrderTest, correctBuyPriceTriggersFullMatch) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 5, 50.0);

    uint32_t id;
    ob.insertOrder(id, Side::BUY, 5, 40.0);  // doesn't cross

    Order updated(Side::BUY, 5, 55.0);
    EXPECT_EQ(Status::FULFILLED, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

// Correct a sell to a price that crosses a resting buy → immediate fill
TEST(CorrectOrderTest, correctSellPriceTriggersFullMatch) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 5, 50.0);

    uint32_t id;
    ob.insertOrder(id, Side::SELL, 5, 60.0);  // doesn't cross

    Order updated(Side::SELL, 5, 45.0);
    EXPECT_EQ(Status::FULFILLED, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getBidCount(), 0);
    EXPECT_EQ(ob.getAskCount(), 0);
}

// Correct to a crossing price where only partial liquidity is available
TEST(CorrectOrderTest, correctPriceTriggersPartialMatch) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 3, 50.0);

    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 40.0);

    Order updated(Side::BUY, 10, 55.0);
    EXPECT_EQ(Status::PARTIALLY_FULFILLED, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getAskCount(), 0);
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(55.0).peek().qty, 7);
}

// Correcting a fulfilled order (not in book) should return ERROR
TEST(CorrectOrderTest, correctFulfilledOrderReturnsError) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 5, 50.0);

    uint32_t id;
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(id, Side::BUY, 5, 50.0));

    Order updated(Side::BUY, 5, 50.0);
    EXPECT_EQ(Status::ERROR, ob.correctOrder(id, updated));
}

// Correcting a partially-filled resting order replaces it with the new qty
TEST(CorrectOrderTest, correctPartiallyFilledOrder) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 3, 50.0);

    uint32_t id;
    ASSERT_EQ(Status::PARTIALLY_FULFILLED, ob.insertOrder(id, Side::BUY, 10, 55.0));
    // 7 remain in the book at 55.0

    Order updated(Side::BUY, 5, 50.0);
    EXPECT_EQ(Status::PENDING, ob.correctOrder(id, updated));

    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getBidPriceLevel(50.0).peek().qty, 5);
}

// Sequential corrections on the same order all succeed
TEST(CorrectOrderTest, correctMultipleTimesSequentially) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);

    Order u1(Side::BUY, 8, 51.0);
    EXPECT_EQ(Status::PENDING, ob.correctOrder(id, u1));
    EXPECT_EQ(ob.getHighestBid(), 51.0);

    Order u2(Side::BUY, 6, 52.0);
    EXPECT_EQ(Status::PENDING, ob.correctOrder(id, u2));
    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getHighestBid(), 52.0);
    EXPECT_EQ(ob.getBidPriceLevel(52.0).peek().qty, 6);
}

// ==================== getAllRestingOrders Tests ====================

TEST(GetAllRestingOrdersTest, emptyBookReturnsEmpty) {
    Orderbook ob;
    EXPECT_TRUE(ob.getAllRestingOrders().empty());
}

TEST(GetAllRestingOrdersTest, singleBuyOrder) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);

    auto ids = ob.getAllRestingOrders();
    ASSERT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], id);
}

TEST(GetAllRestingOrdersTest, singleSellOrder) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::SELL, 5, 60.0);

    auto ids = ob.getAllRestingOrders();
    ASSERT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], id);
}

TEST(GetAllRestingOrdersTest, mixedBuysAndSells) {
    Orderbook ob;
    uint32_t id1, id2, id3;
    ob.insertOrder(id1, Side::BUY, 10, 50.0);
    ob.insertOrder(id2, Side::BUY, 5, 49.0);
    ob.insertOrder(id3, Side::SELL, 8, 60.0);

    auto ids = ob.getAllRestingOrders();
    ASSERT_EQ(ids.size(), 3);

    std::unordered_set<uint32_t> idSet(ids.begin(), ids.end());
    EXPECT_TRUE(idSet.count(id1));
    EXPECT_TRUE(idSet.count(id2));
    EXPECT_TRUE(idSet.count(id3));
}

TEST(GetAllRestingOrdersTest, fulfilledOrderNotIncluded) {
    Orderbook ob;
    ob.insertOrder(Side::SELL, 5, 50.0);

    uint32_t buyId;
    ASSERT_EQ(Status::FULFILLED, ob.insertOrder(buyId, Side::BUY, 5, 50.0));

    EXPECT_TRUE(ob.getAllRestingOrders().empty());
}

TEST(GetAllRestingOrdersTest, cancelledOrderNotIncluded) {
    Orderbook ob;
    uint32_t id;
    ob.insertOrder(id, Side::BUY, 10, 50.0);
    ob.cancelOrder(id);

    EXPECT_TRUE(ob.getAllRestingOrders().empty());
}

TEST(GetAllRestingOrdersTest, countMatchesBidPlusAskCount) {
    Orderbook ob;
    ob.insertOrder(Side::BUY, 10, 50.0);
    ob.insertOrder(Side::BUY, 5, 49.0);
    ob.insertOrder(Side::SELL, 8, 60.0);
    ob.insertOrder(Side::SELL, 3, 61.0);

    EXPECT_EQ(ob.getAllRestingOrders().size(), ob.getBidCount() + ob.getAskCount());
}

TEST(GetAllRestingOrdersTest, partialFillLeavesRestingOrder) {
    Orderbook ob;
    uint32_t sellId;
    ob.insertOrder(sellId, Side::SELL, 10, 50.0);
    ob.insertOrder(Side::BUY, 4, 50.0);  // partially fills sell

    auto ids = ob.getAllRestingOrders();
    ASSERT_EQ(ids.size(), 1);
    EXPECT_EQ(ids[0], sellId);
}

// Correcting one order must not affect other orders at the same price level
TEST(CorrectOrderTest, correctDoesNotAffectOtherOrders) {
    Orderbook ob;
    uint32_t id1, id2;
    ob.insertOrder(id1, Side::BUY, 10, 50.0);
    ob.insertOrder(id2, Side::BUY, 5, 50.0);

    Order updated(Side::BUY, 10, 55.0);
    ob.correctOrder(id1, updated);

    // id2 still at 50.0 with original qty
    EXPECT_EQ(ob.getBidCount(), 2);
    EXPECT_EQ(ob.getBidPriceLevel(50.0).peek().qty, 5);
    EXPECT_EQ(ob.getBidPriceLevel(55.0).peek().qty, 10);
}