#include "logger.h"
#include <filesystem>

namespace orderbook {

Logger::Logger(const LogLevel level) : d_level(level) {
    if (d_level != LogLevel::OFF) {
        std::filesystem::create_directories("logs");
        d_file.open("logs/orderbook.log", std::ios::trunc);
    }
}

Logger::~Logger() {
    if (d_file.is_open()) {
        d_file.close();
    }
}

void Logger::printEvent(const Order& order) {
    if (d_level == LogLevel::OFF || !d_file.is_open()) return;

    d_file << "(" << now() << ")[ORDER]: {Side: "
           << (order.side == Side::BUY ? "BUY" : "SELL")
           << ", qty: " << order.qty
           << ", price: " << order.price
           << ", id: " << order.id
           << "}" << std::endl;
}

void Logger::printEvent(Order& buyer, Order& seller, uint32_t qty, double price) {
    if (d_level == LogLevel::OFF || !d_file.is_open()) return;

    d_file << "(" << now() << ")[TRADE]: {qty: " << qty
           << ", price: " << price
           << ", buyerId: " << buyer.id
           << ", sellerId: " << seller.id
           << "}" << std::endl;
}

void Logger::printCancel(const Order& order) {
    if (d_level == LogLevel::OFF || !d_file.is_open()) return;

    d_file << "(" << now() << ")[CANCEL]: {id: " << order.id
           << ", price: " << order.price
           << ", qty: " << order.qty
           << "}" << std::endl;
}

} // namespace orderbook
