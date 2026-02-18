#ifndef PRICELEVEL_H
#define PRICELEVEL_H

#include <cstdint>
#include <queue>
#include "booktypes.h"

namespace orderbook {

class PriceLevel {
public:
    PriceLevel();
    PriceLevel(double price);

    bool add(const Order& order);
    const bool pop();
    Order& peek();

    const size_t getSize();
    const uint32_t getQty();
    const double getPrice();

    PriceLevel& operator=(const PriceLevel& other);

private:
    double d_price;
    uint32_t d_qty;
    std::queue<Order> d_q;
};

} // namespace orderbook
#endif // PRICELEVEL_H