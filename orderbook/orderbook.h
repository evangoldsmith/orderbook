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

    BookResponse insertOrder(Side side, uint32_t qty, double price);

    double getHighestBid() const;
    double getLowestAsk() const;
    size_t getBidCount() const;
    size_t getAskCount() const;
    PriceLevel& getAskPriceLevel(double price);
    PriceLevel& getBidPriceLevel(double price);

private:
    void addToBooks(const Order& order);
    void createTrade(Order& buyer, Order& seller, uint32_t qty);
    bool processBidMatch(Order& order);
    bool processAskMatch(Order& order);

    std::map<double, PriceLevel> d_bids;
    std::map<double, PriceLevel> d_asks;
};

} // namespace orderbook
#endif // ORDERBOOK_H