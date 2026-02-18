#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <cstdint>
#include <map>
#include "booktypes.h"
#include "pricelevel.h"

namespace orderbook {

class Orderbook {
public:
    Orderbook() = default;

    const BookResponse insertOrder(Side side, uint32_t qty, double price);
    const bool processBidMatch(Order& order);
    const bool processAskMatch(Order& order);

    const double getHighestBid();
    const double getLowestAsk();
    const size_t getBidCount();
    const size_t getAskCount();
    PriceLevel& getAskPriceLevel(const double price);
    PriceLevel& getBidPriceLevel(const double price);

private:
    const void addToBooks(const Order& order);
    const void createTrade(Order& buyer, Order& seller, uint32_t qty);

    std::map<double, PriceLevel> d_bids;
    std::map<double, PriceLevel> d_asks;
};

} // namespace orderbook
#endif // ORDERBOOK_H