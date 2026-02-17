#include <map>
#include "orderbook.h"
#include "booktypes.h"

namespace orderbook {

BookResponse Orderbook::insertOrder(Side side, uint32_t qty, double price) {
    Order newOrder(side, qty, price);
    std::map<double, PriceLevel>& book = (side == Side::BUY) ? d_bids : d_asks;

    if (!book.contains(price)) {
        PriceLevel level(price);
        book[price] = level;
    }
    book[price].add(newOrder);

    return BookResponse::FULFILLED;
}

const size_t Orderbook::getAskCount() {
    size_t count = 0;
    for (auto& [price, level] : d_asks) {
        count += level.getSize();
    }
    return count;
}

const size_t Orderbook::getBidCount() {
    size_t count = 0;
    for (auto& [price, level] : d_bids) {
        count += level.getSize();
    }
    return count;
}

} // namespace orderbook
