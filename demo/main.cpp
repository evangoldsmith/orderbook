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
    const int numEvents = 500;
    const int delayMs = 100;

    for (int i = 0; i < numEvents; i++) {
        int action = actionDist(rng);
        std::vector<uint32_t> resting = ob.getAllRestingOrders();
        if (resting.empty()) action = 0;

        if (action <= 2) {
            Side side = sideDist(rng) == 0 ? Side::BUY : Side::SELL;
            uint32_t qty = qtyDist(rng);
            double price = std::round((midPrice + priceDist(rng)) * 100) / 100;
            uint32_t id;
            Status response = ob.insertOrder(id, side, qty, price);

        } else if (action == 3) {
            std::uniform_int_distribution<size_t> idxDist(0, resting.size() - 1);
            uint32_t targetId = resting[idxDist(rng)];
            bool ok = ob.cancelOrder(targetId);
        } else {
            std::uniform_int_distribution<size_t> idxDist(0, resting.size() - 1);
            uint32_t targetId = resting[idxDist(rng)];

            Side newSide = sideDist(rng) == 0 ? Side::BUY : Side::SELL;
            uint32_t newQty = qtyDist(rng);
            double newPrice = std::round((midPrice + priceDist(rng)) * 100) / 100;

            Order updated(newSide, newQty, newPrice);
            Status response = ob.correctOrder(targetId, updated);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

    }
    return 0;
}
