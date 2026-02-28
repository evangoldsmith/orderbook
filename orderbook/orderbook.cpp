#include <map>
#include <cmath>
#include "orderbook.h"
#include "booktypes.h"

namespace orderbook {

// Process a new order, run matching, and add to book if necessary.
// Returns Status (ERROR, PENDING, PARTIALLY_FULFILLED, FULFILLED)
Status Orderbook::insertOrder(uint32_t& orderId, Side side, uint32_t qty, double price) {
    if (qty <= 0 || price <= 0) {
        return Status::ERROR;
    }

    // Attempt to match order against book
    Order newOrder(side, qty, price);
    orderId = newOrder.id;

    return processOrder(newOrder);
}

Status Orderbook::processOrder(Order& order) {
    d_logger.printEvent(order);
    const uint32_t origQty = order.qty;
    Status res = Status::PENDING;
    if (tryMatch(order)) {
        res = Status::FULFILLED;
    } else if (order.qty != origQty) {
        res = Status::PARTIALLY_FULFILLED;
    }

    // Add full or remaining order to book
    order.status = res;
    if (res != Status::FULFILLED) { addToBooks(order); }
    return res;
}


Status Orderbook::insertOrder(Side side, uint32_t qty, double price) {
    if (qty <= 0 || price <= 0) {
        return Status::ERROR;
    }

    // Attempt to match order against book
    Order newOrder(side, qty, price);    
    return processOrder(newOrder);
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

// Match an incoming buy order against resting sell orders.
// Returns true if the order is fully filled.
bool Orderbook::processPriceTimeBidMatch(Order& order) {
    while (order.qty > 0 && !d_asks.empty() && order.price >= getLowestAsk()) {
        PriceLevel& level = d_asks.begin()->second;
        Order& resting = level.peek();

        uint32_t fillQty = std::min(order.qty, resting.qty);
        createTrade(order, resting, fillQty);
        level.subtractQty(fillQty);

        if (resting.qty == 0) {
            level.pop();
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
        Order& resting = level.peek();

        uint32_t fillQty = std::min(order.qty, resting.qty);
        createTrade(order, resting, fillQty);
        level.subtractQty(fillQty);

        if (resting.qty == 0) {
            level.pop();
        }

        if (level.getSize() == 0) {
            // Amortized constant removal of last element in std::map
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
        for (Order& restingOrder : level.getQ()) {
            uint32_t share = static_cast<uint32_t>(
                static_cast<double>(restingOrder.qty) / totalQty * fillQty
            );

            createTrade(order, restingOrder, share);
            filled += share;
        }

        // Distribute reaminder by FIFO
        uint32_t remainingQty = fillQty - filled;
        for (Order& restingOrder : level.getQ()) {
            uint32_t share = std::min(remainingQty, restingOrder.qty);

            createTrade(order, restingOrder, share);
            remainingQty -= share;
        }

        level.subtractQty(fillQty);
        level.clearEmptyOrders();
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
        for (Order& restingOrder : level.getQ()) {
            uint32_t share = static_cast<uint32_t>(
                static_cast<double>(restingOrder.qty) / totalQty * fillQty
            );

            createTrade(order, restingOrder, share);
            filled += share;    
        }

        // Distribute reaminder by FIFO
        uint32_t remainingQty = fillQty - filled;
        for (Order& restingOrder : level.getQ()) {
            uint32_t share = std::min(remainingQty, restingOrder.qty);

            createTrade(order, restingOrder, share);
            remainingQty -= share;
        }

        level.subtractQty(fillQty);
        level.clearEmptyOrders();
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

Status Orderbook::correctOrder(uint32_t orderId, Order& updatedOrder) {
    if (!d_orders.contains(orderId)) {
        return Status::ERROR; // TODO: Add exceptions
    }

    // Remove original trade
    if (!cancelOrder(orderId)) {
        return Status::ERROR;
    }
    
    // Process updated order with the same ID
    Order newOrder(orderId, updatedOrder.side, updatedOrder.qty, updatedOrder.price);
    return processOrder(newOrder);
}


// Add order to respective book (bids/asks) depending on buy/sell
void Orderbook::addToBooks(const Order& order) {
    std::map<double, PriceLevel>& book = (order.side == Side::BUY) ? d_bids : d_asks;

    // Add order to price level
    if (!book.contains(order.price)) {
        PriceLevel level(order.price);
        book[order.price] = level;
    }
    book[order.price].add(order);

    // Add order to ID reference map
    std::list<Order>& priceLevelOrders = book[order.price].getQ();
    OrderEntry it = std::prev(priceLevelOrders.end());
    d_orders.insert({order.id, it});
}

bool Orderbook::cancelOrder(uint32_t orderId) {
    if (!d_orders.contains(orderId)) {
        return false; // TODO: Add exceptions
    }

    // Remove from price level
    Order order = getOrder(orderId);
    std::map<double, PriceLevel>& book = (order.side == Side::BUY) ? d_bids : d_asks;
    PriceLevel& level = book[order.price];
    OrderEntry it = d_orders.at(orderId);
    if (!level.remove(it)) {
        return false; // TODO: Add exceptions
    }
    if (level.getSize() == 0) {
        book.erase(order.price);
    }

    // Remove from order map
    d_orders.erase(orderId);
    d_logger.printCancel(order);

    return true;
}

const Order& Orderbook::getOrder(uint32_t orderId) const {
    return *d_orders.at(orderId);
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
