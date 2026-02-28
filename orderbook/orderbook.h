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

    Status insertOrder(uint32_t& orderId, Side side, uint32_t qty, double price);
    Status insertOrder(Side side, uint32_t qty, double price);
    Status correctOrder(uint32_t orderId, Order& updatedOrder);
    bool cancelOrder(uint32_t orderId);

    double getHighestBid() const;
    double getLowestAsk() const;
    size_t getBidCount() const;
    size_t getAskCount() const;
    PriceLevel& getAskPriceLevel(double price);
    PriceLevel& getBidPriceLevel(double price);
    const Order& getOrder(uint32_t orderId) const;

private:
    bool tryMatch(Order& order);
    void addToBooks(const Order& order);
    void createTrade(Order& aggressor, Order& resting, uint32_t qty);
    Status processOrder(Order& order);
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