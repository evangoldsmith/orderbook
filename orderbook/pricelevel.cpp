#include "pricelevel.h"

namespace orderbook {

PriceLevel::PriceLevel() : d_price(0), d_qty(0) {}

PriceLevel::PriceLevel(double price) : d_price(price), d_qty(0) {}

// Add new order to price level
bool PriceLevel::add(const Order& order) {
    d_q.push(order);
    d_qty += order.qty;
    return true;
}

// Remove the first order in the queue, return false on empty.
const bool PriceLevel::pop() {
    if (!d_q.empty()) {
        d_qty -= d_q.front().qty;
        d_q.pop();
        return true;
    }
    return false;
}

Order& PriceLevel::peek() { return d_q.front(); }

const size_t PriceLevel::getSize() { return d_q.size(); }

const uint32_t PriceLevel::getQty() { return d_qty; }

const double PriceLevel::getPrice() { return d_price; }

PriceLevel& PriceLevel::operator=(const PriceLevel& other) {
    if (this != &other) {
        d_price = other.d_price;
        d_qty = other.d_qty;
        d_q = other.d_q;
    }
    return *this;
}

} // namespace orderbook
