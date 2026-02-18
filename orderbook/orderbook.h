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

    const double getHighestBid();
    const double getLowestAsk();
    const size_t getBidCount();
    const size_t getAskCount();

private:
    std::map<double, PriceLevel> d_bids;
    std::map<double, PriceLevel> d_asks;
};

} // namespace orderbook
#endif // ORDERBOOK_H