#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <unordered_set>
#include "orderbook.h"

using namespace orderbook;

int main() {
    Orderbook ob(LogLevel::FULL, MatchingMode::PRICE_TIME);
    std::unordered_set<uint32_t> ids;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<uint32_t> qtyDist(1, 20);
    std::uniform_real_distribution<double> priceDist(-2.0, 2.0);

    const double midPrice = 100.0;
    const int numOrders = 100;
    const int delayMs = 15;

    std::cout << "Sending " << numOrders << " random orders around $"
              << midPrice << "..." << std::endl;
    std::cout << "Logging to logs/orderbook.log" << std::endl;
    std::cout << "---" << std::endl;

    for (int i = 0; i < numOrders; i++) {
        Side side = sideDist(rng) == 0 ? Side::BUY : Side::SELL;
        uint32_t qty = qtyDist(rng);
        double price = std::round((midPrice + priceDist(rng)) * 100) / 100;

        uint32_t id;
        Status response = ob.insertOrder(id, side, qty, price);

        std::cout << "[" << (i + 1) << "] "
                  << (side == Side::BUY ? "BUY " : "SELL")
                  << "id=" << id << " qty=" << qty << " @ $" << price << " -> ";

        switch (response) {
            case Status::PENDING:             std::cout << "PENDING"; break;
            case Status::PARTIALLY_FULFILLED:  std::cout << "PARTIAL"; break;
            case Status::FULFILLED:            std::cout << "FILLED";  break;
            case Status::ERROR:                std::cout << "ERROR";   break;
        }

        std::cout << "  (bids=" << ob.getBidCount()
                  << " asks=" << ob.getAskCount() << ")" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
    }

    std::cout << "---" << std::endl;
    std::cout << "Done. Final state: bids=" << ob.getBidCount()
              << " asks=" << ob.getAskCount() << std::endl;

    return 0;
}
