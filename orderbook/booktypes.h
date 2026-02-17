#ifndef BOOKTYPES_H
#define BOOKTYPES_H

#include <cstdint>

namespace orderbook {

using Timestamp = uint64_t;

enum Side {
    BUY,
    SELL
};

enum BookResponse {
    ERROR, // Failed during processing
    PENDING, // Added to book, no orders FULFILLED
    PARTIALLY_FULFILLED, // Some order qty FULFILLED, rest is added to book
    FULFILLED // Full order was FULFILLED
};

struct Order {
    uint32_t id;
    Side side;
    double price;
    uint32_t qty;
    Timestamp time;

    Order(Side s, uint32_t q, double p);
};

} // namespace orderbook
#endif // BOOKTYPES_H