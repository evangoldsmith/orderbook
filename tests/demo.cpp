#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include "orderbook.h"

using namespace orderbook;

int main() {
    Orderbook ob(LogLevel::FULL, MatchingMode::PRICE_TIME);

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<uint32_t> qtyDist(1, 20);
    std::uniform_real_distribution<double> priceDist(-2.0, 2.0);
    std::uniform_int_distribution<int> actionDist(0, 4); // 0-2: insert, 3: cancel, 4: correct

    const double midPrice = 100.0;
    const int numEvents = 100;
    const int delayMs = 15;

    std::cout << "Sending " << numEvents << " random events around $"
              << midPrice << "..." << std::endl;
    std::cout << "Logging to logs/orderbook.log" << std::endl;
    std::cout << "---" << std::endl;

    for (int i = 0; i < numEvents; i++) {
        int action = actionDist(rng);
        std::vector<uint32_t> resting = ob.getAllRestingOrders();

        // Fall back to insert if there are no resting orders to act on
        if (resting.empty()) action = 0;

        if (action <= 2) {
            // Insert
            Side side = sideDist(rng) == 0 ? Side::BUY : Side::SELL;
            uint32_t qty = qtyDist(rng);
            double price = std::round((midPrice + priceDist(rng)) * 100) / 100;

            uint32_t id;
            Status response = ob.insertOrder(id, side, qty, price);

            std::cout << "[" << (i + 1) << "] INSERT "
                      << (side == Side::BUY ? "BUY " : "SELL")
                      << "id=" << id << " qty=" << qty << " @ $" << price << " -> ";

            switch (response) {
                case Status::PENDING:             std::cout << "PENDING"; break;
                case Status::PARTIALLY_FULFILLED: std::cout << "PARTIAL"; break;
                case Status::FULFILLED:           std::cout << "FILLED";  break;
                case Status::ERROR:               std::cout << "ERROR";   break;
            }

        } else if (action == 3) {
            // Cancel a random resting order
            std::uniform_int_distribution<size_t> idxDist(0, resting.size() - 1);
            uint32_t targetId = resting[idxDist(rng)];

            bool ok = ob.cancelOrder(targetId);
            std::cout << "[" << (i + 1) << "] CANCEL id=" << targetId
                      << " -> " << (ok ? "OK" : "FAILED");

        } else {
            // Correct a random resting order
            std::uniform_int_distribution<size_t> idxDist(0, resting.size() - 1);
            uint32_t targetId = resting[idxDist(rng)];

            Side newSide = sideDist(rng) == 0 ? Side::BUY : Side::SELL;
            uint32_t newQty = qtyDist(rng);
            double newPrice = std::round((midPrice + priceDist(rng)) * 100) / 100;

            Order updated(newSide, newQty, newPrice);
            Status response = ob.correctOrder(targetId, updated);

            std::cout << "[" << (i + 1) << "] CORRECT id=" << targetId
                      << " -> " << (newSide == Side::BUY ? "BUY " : "SELL")
                      << "qty=" << newQty << " @ $" << newPrice << " -> ";

            switch (response) {
                case Status::PENDING:             std::cout << "PENDING"; break;
                case Status::PARTIALLY_FULFILLED: std::cout << "PARTIAL"; break;
                case Status::FULFILLED:           std::cout << "FILLED";  break;
                case Status::ERROR:               std::cout << "ERROR";   break;
            }
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
