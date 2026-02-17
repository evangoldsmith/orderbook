#include <gtest/gtest.h>
#include "orderbook.h"

using namespace orderbook;

TEST(OrderbookTest, Construction) {
    Orderbook ob;
    EXPECT_TRUE(true);
}

TEST(OrderbookTest, addBuyOrder) {
    Orderbook ob;

    ob.insertOrder(Side::BUY, 3, 10.0);

    EXPECT_EQ(ob.getBidCount(), 1);
    EXPECT_EQ(ob.getAskCount(), 0);
}

TEST(OrderbookTest, addSellOrder) {
    Orderbook ob;

    ob.insertOrder(Side::SELL, 3, 10.0);
    
    EXPECT_EQ(ob.getAskCount(), 1);
    EXPECT_EQ(ob.getBidCount(), 0);

}