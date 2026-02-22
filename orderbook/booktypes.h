#ifndef BOOKTYPES_H
#define BOOKTYPES_H

#include <cstdint>
#include <chrono>
#include <ostream>

namespace orderbook {

using Timestamp = uint64_t;
inline Timestamp now() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

inline uint32_t nextOrderId() {
    static uint32_t id = 0;
    return ++id;
}

enum class Side {
    BUY,
    SELL
};

enum class Status {
    ERROR, // Failed during processing
    PENDING, // Added to book, no orders FULFILLED
    PARTIALLY_FULFILLED, // Some order qty FULFILLED, rest is added to book
    FULFILLED // Full order was FULFILLED
};

inline std::ostream& operator<<(std::ostream& os, const Status& s) {
    switch (s) {
        case Status::ERROR:               return os << "ERROR";
        case Status::PENDING:             return os << "PENDING";
        case Status::PARTIALLY_FULFILLED: return os << "PARTIALLY_FULFILLED";
        case Status::FULFILLED:           return os << "FULFILLED";
    }
    return os;
}

enum class MatchingMode {
    PRICE_TIME, // FIFO matching
    PRO_RATA // Matching proportional to order volume
};

enum class LogLevel {
    OFF, // No logging
    FULL, // Default order and trade logs
    DEBUG // Order, trade, and system performance logs
};

struct Order {
    uint32_t id;
    Side side;
    double price;
    uint32_t qty;
    Timestamp time;
    Status status;

    Order() : id(0), side(Side::BUY), price(0), qty(0), time(0), status(Status::PENDING) {}
    Order(Side s, uint32_t q, double p)
        : id(nextOrderId()), side(s), price(p), qty(q), time(now()), status(Status::PENDING) {}

    friend std::ostream& operator<<(std::ostream& os, const Order& o) {
        os << "Order{id=" << o.id
           << ", side=" << (o.side == Side::BUY ? "BUY" : "SELL")
           << ", price=" << o.price
           << ", qty=" << o.qty
           << ", time=" << o.time << "}";
        return os;
    }
};

} // namespace orderbook
#endif // BOOKTYPES_H