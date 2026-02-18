#ifndef BOOKTYPES_H
#define BOOKTYPES_H

#include <cstdint>
#include <chrono>
#include <ostream>

namespace orderbook {

using Timestamp = uint64_t;
static uint32_t orderId = 0;

inline Timestamp now() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline uint32_t nextOrderId() {
    return orderId++;
}

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

    Order(Side s, uint32_t q, double p)
        : id(nextOrderId()), side(s), price(p), qty(q), time(now()) {}

    friend std::ostream& operator<<(std::ostream& os, const Order& o) {
        os << "Order{id=" << o.id
           << ", side=" << (o.side == BUY ? "BUY" : "SELL")
           << ", price=" << o.price
           << ", qty=" << o.qty
           << ", time=" << o.time << "}";
        return os;
    }
};

} // namespace orderbook
#endif // BOOKTYPES_H