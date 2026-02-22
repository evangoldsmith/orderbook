#include <gtest/gtest.h>
#include "orderbook.h"
#include "booktypes.h"

using namespace orderbook;

TEST(LoggerTest, construction) {
    Orderbook ob(LogLevel::DEBUG);

    EXPECT_TRUE(true);
}