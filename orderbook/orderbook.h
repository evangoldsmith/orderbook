#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <cstdint>
#include <map>
#include "booktypes.h"
#include "pricelevel.h"

namespace orderbook {

class Orderbook {
public:
    Orderbook(const MatchingMode mode = MatchingMode::PRICE_TIME) : d_matchingMode(mode) {}

    BookResponse insertOrder(Side side, uint32_t qty, double price);

    double getHighestBid() const;
    double getLowestAsk() const;
    size_t getBidCount() const;
    size_t getAskCount() const;
    PriceLevel& getAskPriceLevel(double price);
    PriceLevel& getBidPriceLevel(double price);

private:
    bool tryMatch(Order& order);
    void addToBooks(const Order& order);
    void createTrade(Order& buyer, Order& seller, uint32_t qty);
    bool processPriceTimeBidMatch(Order& order);
    bool processPriceTimeAskMatch(Order& order);
    bool processProRataBidMatch(Order& order);
    bool processProRataAskMatch(Order& order);

    std::map<double, PriceLevel> d_bids;
    std::map<double, PriceLevel> d_asks;
    const MatchingMode d_matchingMode;
};

} // namespace orderbook
#endif // ORDERBOOK_H