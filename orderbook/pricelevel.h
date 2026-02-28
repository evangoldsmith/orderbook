#ifndef PRICELEVEL_H
#define PRICELEVEL_H

#include <cstdint>
#include <deque>
#include <list>
#include <memory>
#include "booktypes.h"

namespace orderbook {

class PriceLevel {
public:
    PriceLevel();
    PriceLevel(double price);

    bool add(const Order& order);
    bool pop();
    bool remove(const OrderEntry order);
    Order& peek();
    void subtractQty(const uint32_t qty);
    void clearEmptyOrders();

    size_t getSize() const;
    uint32_t getQty() const;
    double getPrice() const;
    std::list<Order>& getQ(); // TODO: get rid of this, keep pricelevel fully abstracted

private:
    double d_price;
    uint32_t d_qty;
    std::list<Order> d_orders;
};

} // namespace orderbook
#endif // PRICELEVEL_H