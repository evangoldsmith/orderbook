#include "orderbook.h"
#include "booktypes.h"

namespace orderbook {

BookResponse Orderbook::insertOrder(Side side, uint32_t qty, double price) {
    if (side == Side::BUY) {
        Order newOrder(side, qty, price);
        if (d_bids.contains(price)) {
            d_bids[price].add(newOrder);
        } else {
            PriceLevel level(price);
            d_bids[price] = level;
        }
    } else {

    }

    return BookResponse::FULFILLED;
}

} // namespace orderbook
