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
bool PriceLevel::pop() {
    if (!d_q.empty()) {
        d_qty -= d_q.front().qty;
        d_q.pop();
        return true;
    }
    return false;
}

Order& PriceLevel::peek() { return d_q.front(); }

size_t PriceLevel::getSize() const { return d_q.size(); }

uint32_t PriceLevel::getQty() const { return d_qty; }

double PriceLevel::getPrice() const { return d_price; }

void PriceLevel::subtractQty(const uint32_t qty) { d_qty -= qty; }

} // namespace orderbook
