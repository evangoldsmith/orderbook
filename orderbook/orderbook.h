#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <cstdint>
#include <map>
#include "booktypes.h"
#include "pricelevel.h"

namespace orderbook {

class Orderbook {
public:
    Orderbook() = default;

    BookResponse insertOrder(Side side, uint32_t qty, double price);

private:
    std::map<double, PriceLevel> d_bids;
    std::map<double, PriceLevel> d_asks;
};

} // namespace orderbook
#endif // ORDERBOOK_H