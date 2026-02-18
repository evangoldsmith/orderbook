#include <map>
#include "orderbook.h"
#include "booktypes.h"

namespace orderbook {

// Process a new order, run matching, and add to book if necessary.
// Returns BookResponse (ERROR, PENDING, PARTIALLY_FULFILLED, FULFILLED)
const BookResponse Orderbook::insertOrder(Side side, uint32_t qty, double price) {
    Order newOrder(side, qty, price);
    BookResponse status = PARTIALLY_FULFILLED;

    if (side == BUY && !d_asks.empty()) {
        if (processBidMatch(newOrder)) {
            return BookResponse::FULFILLED;
        }
    } else if (side == SELL && !d_bids.empty()) {
        if (processAskMatch(newOrder)) {
            return BookResponse::FULFILLED;
        }
    } else {
        status = PENDING;
    }

    addToBooks(newOrder);
    return status;
}

// Match an incoming buy order against resting sell orders.
// Returns true if the order is fully filled.
const bool Orderbook::processBidMatch(Order& order) {
    while (order.qty > 0 && !d_asks.empty() && order.price >= getLowestAsk()) {
        double askPrice = getLowestAsk();
        PriceLevel& level = d_asks[askPrice];
        Order& resting = level.peek();

        uint32_t fillQty = std::min(order.qty, resting.qty);
        createTrade(order, resting, fillQty);

        if (resting.qty == 0) {
            level.pop();
        }

        if (level.getSize() == 0) {
            d_asks.erase(askPrice);
        }
    }

    return order.qty == 0;
}

const bool Orderbook::processAskMatch(Order& order) {
    while (order.qty > 0 && !d_bids.empty() && order.price <= getHighestBid()) {
        double bidPrice = getHighestBid();
        PriceLevel& level = d_bids[bidPrice];
        Order& resting = level.peek();

        uint32_t fillQty = std::min(order.qty, resting.qty);
        createTrade(resting, order, fillQty);

        if (resting.qty == 0) {
            level.pop();
        }

        if (level.getSize() == 0) {
            d_bids.erase(bidPrice);
        }
    }

    return order.qty == 0;
}

const void Orderbook::createTrade(Order& buyer, Order& seller, uint32_t qty) {
    buyer.qty -= qty;
    seller.qty -= qty;
}

// Add order to respective book (bids/asks) depending on buy/sell
const void Orderbook::addToBooks(const Order& order) {
    std::map<double, PriceLevel>& book = (order.side == Side::BUY) ? d_bids : d_asks;

    if (!book.contains(order.price)) {
        PriceLevel level(order.price);
        book[order.price] = level;
    }
    book[order.price].add(order);
}

// Number of total sell orders in the book
const size_t Orderbook::getAskCount() {
    size_t count = 0;
    for (auto& [price, level] : d_asks) {
        count += level.getSize();
    }
    return count;
}

// Number of total buy orders in the book
const size_t Orderbook::getBidCount() {
    size_t count = 0;
    for (auto& [price, level] : d_bids) {
        count += level.getSize();
    }
    return count;
}

// Highest priced bid order in the book
const double Orderbook::getHighestBid() {
    if (!d_bids.empty()) {
        return d_bids.rbegin()->first;
    }
    return 0.0;
}

// Lowest priced sell offer in the book
const double Orderbook::getLowestAsk() {
    if (!d_asks.empty()) {
        return d_asks.begin()->first;
    }
    return 0.0;
}

PriceLevel& Orderbook::getAskPriceLevel(const double price) {
    return d_asks[price];
}

PriceLevel& Orderbook::getBidPriceLevel(const double price) {
    return d_bids[price];
}


} // namespace orderbook
