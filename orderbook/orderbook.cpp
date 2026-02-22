#include <map>
#include <cmath>
#include "orderbook.h"
#include "booktypes.h"

namespace orderbook {

// Process a new order, run matching, and add to book if necessary.
// Returns Status (ERROR, PENDING, PARTIALLY_FULFILLED, FULFILLED)
Status Orderbook::insertOrder(Side side, uint32_t qty, double price) {
    if (qty <= 0 || price <= 0) {
        return Status::ERROR;
    }

    Order newOrder(side, qty, price);
    d_logger.printEvent(newOrder);
    Status res = Status::PENDING;
    if (tryMatch(newOrder)) {
        res = Status::FULFILLED;
    } else if (newOrder.qty != qty) {
        res = Status::PARTIALLY_FULFILLED;
    }

    newOrder.status = res;
    if (res != Status::FULFILLED) {
        d_orders[newOrder.id] = newOrder;
        addToBooks(newOrder.id, newOrder);
    }
    return res;
}

bool Orderbook::tryMatch(Order& order) {
    if (order.side == Side::BUY && !d_asks.empty()) {
        if (d_matchingMode == MatchingMode::PRO_RATA) {
            return processProRataBidMatch(order);
        }
        return processPriceTimeBidMatch(order);
    }
    if (order.side == Side::SELL && !d_bids.empty()) {
        if (d_matchingMode == MatchingMode::PRO_RATA) {
            return processProRataAskMatch(order);
        }
        return processPriceTimeAskMatch(order);
    }

    return false;
}

bool Orderbook::cancelOrder(uint32_t orderId) {
    auto it = d_orders.find(orderId);
    if (it == d_orders.end()) {
        return false;
    }

    Order& order = it->second;
    std::map<double, PriceLevel>& book = (order.side == Side::BUY) ? d_bids : d_asks;

    auto levelIt = book.find(order.price);
    if (levelIt != book.end()) {
        levelIt->second.subtractQty(order.qty);
        if (levelIt->second.getQty() == 0) {
            book.erase(levelIt);
        }
    }

    d_orders.erase(it);
    return true;
}

// Match an incoming buy order against resting sell orders.
// Returns true if the order is fully filled.
bool Orderbook::processPriceTimeBidMatch(Order& order) {
    while (order.qty > 0 && !d_asks.empty() && order.price >= getLowestAsk()) {
        PriceLevel& level = d_asks.begin()->second;

        // Lazy skip: cancelled orders still in deque
        while (level.getSize() > 0 && d_orders.find(level.front()) == d_orders.end()) {
            level.pop();
        }
        if (level.getSize() == 0) {
            d_asks.erase(d_asks.begin());
            continue;
        }

        uint32_t restingId = level.front();
        Order& resting = d_orders[restingId];

        uint32_t fillQty = std::min(order.qty, resting.qty);
        createTrade(order, resting, fillQty);
        level.subtractQty(fillQty);

        if (resting.qty == 0) {
            level.pop();
            d_orders.erase(restingId);
        }

        if (level.getSize() == 0) {
            d_asks.erase(d_asks.begin());
        }
    }

    return order.qty == 0;
}

bool Orderbook::processPriceTimeAskMatch(Order& order) {
    while (order.qty > 0 && !d_bids.empty() && order.price <= getHighestBid()) {
        PriceLevel& level = d_bids.rbegin()->second;

        // Lazy skip: cancelled orders still in deque
        while (level.getSize() > 0 && d_orders.find(level.front()) == d_orders.end()) {
            level.pop();
        }
        if (level.getSize() == 0) {
            d_bids.erase(std::prev(d_bids.end()));
            continue;
        }

        uint32_t restingId = level.front();
        Order& resting = d_orders[restingId];

        uint32_t fillQty = std::min(order.qty, resting.qty);
        createTrade(order, resting, fillQty);
        level.subtractQty(fillQty);

        if (resting.qty == 0) {
            level.pop();
            d_orders.erase(restingId);
        }

        if (level.getSize() == 0) {
            d_bids.erase(std::prev(d_bids.end()));
        }
    }

    return order.qty == 0;
}

// Pro-Rata proportional matching algorithm
bool Orderbook::processProRataBidMatch(Order& order) {
    while (order.qty > 0 && !d_asks.empty() && order.price >= getLowestAsk()) {
        PriceLevel& level = d_asks.begin()->second;
        uint32_t totalQty = level.getQty();
        uint32_t fillQty = std::min(order.qty, totalQty);

        // Calculate proportional shares
        size_t filled = 0;
        for (uint32_t id : level.getIds()) {
            if (d_orders.find(id) == d_orders.end()) continue;
            Order& restingOrder = d_orders[id];
            uint32_t share = static_cast<uint32_t>(
                static_cast<double>(restingOrder.qty) / totalQty * fillQty
            );

            createTrade(order, restingOrder, share);
            filled += share;
        }

        // Distribute remainder by FIFO
        uint32_t remainingQty = fillQty - filled;
        for (uint32_t id : level.getIds()) {
            if (remainingQty == 0) break;
            if (d_orders.find(id) == d_orders.end()) continue;
            Order& restingOrder = d_orders[id];
            uint32_t share = std::min(remainingQty, restingOrder.qty);

            createTrade(order, restingOrder, share);
            remainingQty -= share;
        }

        // Clean up zeroed orders
        level.subtractQty(fillQty);
        std::unordered_set<uint32_t> toRemove;
        for (uint32_t id : level.getIds()) {
            auto it = d_orders.find(id);
            if (it != d_orders.end() && it->second.qty == 0) {
                toRemove.insert(id);
                d_orders.erase(it);
            }
        }
        level.removeIds(toRemove);

        if (level.getSize() == 0) {
            d_asks.erase(d_asks.begin());
        }
    }
    return order.qty == 0;
}

bool Orderbook::processProRataAskMatch(Order& order) {
    while (order.qty > 0 && !d_bids.empty() && order.price <= getHighestBid()) {
        PriceLevel& level = d_bids.rbegin()->second;
        uint32_t totalQty = level.getQty();
        uint32_t fillQty = std::min(order.qty, totalQty);

        // Calculate proportional shares
        size_t filled = 0;
        for (uint32_t id : level.getIds()) {
            if (d_orders.find(id) == d_orders.end()) continue;
            Order& restingOrder = d_orders[id];
            uint32_t share = static_cast<uint32_t>(
                static_cast<double>(restingOrder.qty) / totalQty * fillQty
            );

            createTrade(order, restingOrder, share);
            filled += share;
        }

        // Distribute remainder by FIFO
        uint32_t remainingQty = fillQty - filled;
        for (uint32_t id : level.getIds()) {
            if (remainingQty == 0) break;
            if (d_orders.find(id) == d_orders.end()) continue;
            Order& restingOrder = d_orders[id];
            uint32_t share = std::min(remainingQty, restingOrder.qty);

            createTrade(order, restingOrder, share);
            remainingQty -= share;
        }

        // Clean up zeroed orders
        level.subtractQty(fillQty);
        std::unordered_set<uint32_t> toRemove;
        for (uint32_t id : level.getIds()) {
            auto it = d_orders.find(id);
            if (it != d_orders.end() && it->second.qty == 0) {
                toRemove.insert(id);
                d_orders.erase(it);
            }
        }
        level.removeIds(toRemove);

        if (level.getSize() == 0) {
            d_bids.erase(std::prev(d_bids.end()));
        }
    }
    return order.qty == 0;
}

void Orderbook::createTrade(Order& aggressor, Order& resting, uint32_t qty) {
    aggressor.qty -= qty;
    resting.qty -= qty;
    if (aggressor.side == Side::BUY) {
        d_logger.printEvent(aggressor, resting, qty, aggressor.price);
    } else {
        d_logger.printEvent(resting, aggressor, qty, aggressor.price);
    }
}

// Add order to respective book (bids/asks) depending on buy/sell
void Orderbook::addToBooks(uint32_t orderId, const Order& order) {
    std::map<double, PriceLevel>& book = (order.side == Side::BUY) ? d_bids : d_asks;

    if (!book.contains(order.price)) {
        PriceLevel level(order.price);
        book[order.price] = level;
    }
    book[order.price].add(orderId, order.qty);
}

const Order& Orderbook::getOrder(uint32_t id) const {
    return d_orders.at(id);
}

// Number of total sell orders in the book
size_t Orderbook::getAskCount() const {
    size_t count = 0;
    for (auto& [price, level] : d_asks) {
        count += level.getSize();
    }
    return count;
}

// Number of total buy orders in the book
size_t Orderbook::getBidCount() const {
    size_t count = 0;
    for (auto& [price, level] : d_bids) {
        count += level.getSize();
    }
    return count;
}

// Highest priced bid order in the book
double Orderbook::getHighestBid() const {
    if (!d_bids.empty()) {
        return d_bids.rbegin()->first;
    }
    return 0.0;
}

// Lowest priced sell offer in the book
double Orderbook::getLowestAsk() const {
    if (!d_asks.empty()) {
        return d_asks.begin()->first;
    }
    return 0.0;
}

// O(log(n)) accessors
PriceLevel& Orderbook::getAskPriceLevel(double price) {
    return d_asks[price];
}

PriceLevel& Orderbook::getBidPriceLevel(double price) {
    return d_bids[price];
}


} // namespace orderbook
