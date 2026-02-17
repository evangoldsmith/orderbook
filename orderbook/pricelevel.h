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

    bool add(Order order);
    PriceLevel& operator=(const PriceLevel& other);

private:
    const double d_price;
    uint32_t d_qty;
    std::queue<Order> q;
};

} // namespace orderbook
#endif // PRICELEVEL_H