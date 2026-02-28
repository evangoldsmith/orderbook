#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <cstdint>
#include <map>
#include <unordered_map>
#include "booktypes.h"
#include "pricelevel.h"
#include "logger.h"

namespace orderbook {

class Orderbook {
public:
    Orderbook(const MatchingMode mode = MatchingMode::PRICE_TIME) : d_matchingMode(mode) {}
    Orderbook(const LogLevel logLevel, const MatchingMode mode = MatchingMode::PRICE_TIME) : d_matchingMode(mode), d_logger(Logger(logLevel)) {}

    Status insertOrder(Side side, uint32_t qty, double price);

    double getHighestBid() const;
    double getLowestAsk() const;
    size_t getBidCount() const;
    size_t getAskCount() const;
    PriceLevel& getAskPriceLevel(double price);
    PriceLevel& getBidPriceLevel(double price);
    const Order& getOrder(uint32_t orderId) const;
    bool cancelOrder(uint32_t orderId);

private:
    bool tryMatch(Order& order);
    void addToBooks(const Order& order);
    void createTrade(Order& aggressor, Order& resting, uint32_t qty);
    bool processPriceTimeBidMatch(Order& order);
    bool processPriceTimeAskMatch(Order& order);
    bool processProRataBidMatch(Order& order);
    bool processProRataAskMatch(Order& order);

    std::map<double, PriceLevel> d_bids;
    std::map<double, PriceLevel> d_asks;
    std::unordered_map<uint32_t, OrderEntry> d_orders;
    const MatchingMode d_matchingMode;
    Logger d_logger;
};

} // namespace orderbook
#endif // ORDERBOOK_H