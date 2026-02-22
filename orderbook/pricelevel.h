#ifndef PRICELEVEL_H
#define PRICELEVEL_H

#include <cstdint>
#include <deque>
#include <unordered_set>

namespace orderbook {

class PriceLevel {
public:
    PriceLevel();
    PriceLevel(double price);

    void add(uint32_t orderId, uint32_t qty);
    bool pop();
    uint32_t front() const;
    void subtractQty(uint32_t qty);
    void removeIds(const std::unordered_set<uint32_t>& toRemove);

    size_t getSize() const;
    uint32_t getQty() const;
    double getPrice() const;
    std::deque<uint32_t>& getIds();

private:
    double d_price;
    uint32_t d_qty;
    std::deque<uint32_t> d_ids;
};

} // namespace orderbook
#endif // PRICELEVEL_H
