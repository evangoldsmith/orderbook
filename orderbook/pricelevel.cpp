#include "pricelevel.h"

namespace orderbook {

PriceLevel::PriceLevel() : d_price(0), d_qty(0) {}

PriceLevel::PriceLevel(double price) : d_price(price), d_qty(0) {}

// Add new order to price level
bool PriceLevel::add(const Order& order) {
    d_orders.push_back(order);    
    d_qty += order.qty;
    return true;
}

// Remove the first order in the queue, return false on empty.
bool PriceLevel::pop() {
    if (!d_orders.empty()) {
        d_qty -= d_orders.front().qty;
        d_orders.pop_front();
        return true;
    }
    return false;
}

void PriceLevel::clearEmptyOrders() {
    d_orders.erase(
        std::remove_if(d_orders.begin(), d_orders.end(), [](const Order& o) 
        { return o.qty == 0; }), d_orders.end()
    );
}

void PriceLevel::subtractQty(const uint32_t qty) { d_qty -= qty; }

std::list<Order>& PriceLevel::getQ() { return d_orders; }

Order& PriceLevel::peek() { return d_orders.front(); }

size_t PriceLevel::getSize() const { return d_orders.size(); }

uint32_t PriceLevel::getQty() const { return d_qty; }

double PriceLevel::getPrice() const { return d_price; }

} // namespace orderbook
