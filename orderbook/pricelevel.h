#ifndef PRICELEVEL_H
#define PRICELEVEL_H

#include <cstdint>
#include <list>

namespace orderbook {

class PriceLevel {
public:
    PriceLevel();
    PriceLevel(double price);

    std::list<uint32_t>::iterator add(uint32_t orderId, uint32_t qty);
    bool pop();
    uint32_t front() const;
    void subtractQty(uint32_t qty);
    void erase(std::list<uint32_t>::iterator it);

    size_t getSize() const;
    uint32_t getQty() const;
    double getPrice() const;
    std::list<uint32_t>& getIds();

private:
    double d_price;
    uint32_t d_qty;
    std::list<uint32_t> d_ids;
};

} // namespace orderbook
#endif // PRICELEVEL_H
