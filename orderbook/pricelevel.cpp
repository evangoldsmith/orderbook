#include "pricelevel.h"

namespace orderbook {

PriceLevel::PriceLevel() : d_price(0), d_qty(0) {}

PriceLevel::PriceLevel(double price) : d_price(price), d_qty(0) {}

std::list<uint32_t>::iterator PriceLevel::add(uint32_t orderId, uint32_t qty) {
    d_ids.push_back(orderId);
    d_qty += qty;
    return std::prev(d_ids.end());
}

bool PriceLevel::pop() {
    if (!d_ids.empty()) {
        d_ids.pop_front();
        return true;
    }
    return false;
}

uint32_t PriceLevel::front() const { return d_ids.front(); }

void PriceLevel::erase(std::list<uint32_t>::iterator it) {
    d_ids.erase(it);
}

void PriceLevel::subtractQty(uint32_t qty) { d_qty -= qty; }

std::list<uint32_t>& PriceLevel::getIds() { return d_ids; }

size_t PriceLevel::getSize() const { return d_ids.size(); }

uint32_t PriceLevel::getQty() const { return d_qty; }

double PriceLevel::getPrice() const { return d_price; }

} // namespace orderbook
