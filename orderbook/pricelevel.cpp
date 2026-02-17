#include "pricelevel.h"

namespace orderbook {

PriceLevel::PriceLevel() : d_price(0), d_qty(0) {}

PriceLevel::PriceLevel(double price) : d_price(price), d_qty(0) {}

bool PriceLevel::add(Order order) {
    q.push(order);
    d_qty += order.qty;
    return true;
}

const size_t PriceLevel::getSize() { return q.size(); }

const uint32_t PriceLevel::getQty() { return d_qty; }

const double PriceLevel::getPrice() { return d_price; }

PriceLevel& PriceLevel::operator=(const PriceLevel& other) {
    if (this != &other) {
        d_qty = other.d_qty;
        q = other.q;
    }
    return *this;
}

} // namespace orderbook
