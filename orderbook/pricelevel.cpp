#include "pricelevel.h"

namespace orderbook {

PriceLevel::PriceLevel() : d_price(0), d_qty(0) {}

PriceLevel::PriceLevel(double price) : d_price(price), d_qty(0) {}

bool PriceLevel::add(const Order& order) {
    d_q.push(order);
    d_qty += order.qty;
    return true;
}

const bool PriceLevel::pop(Order& order) {
    if (!d_q.empty()) {
        order = d_q.front();
        d_qty -= order.qty;
        d_q.pop();
        return true;
    }
    return false;
}

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
