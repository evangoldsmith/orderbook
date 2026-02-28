#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include "booktypes.h"

namespace orderbook {

class Logger {
public:
    Logger(const LogLevel level = LogLevel::OFF);
    ~Logger();

    void printEvent(const Order& order);
    void printEvent(Order& buyer, Order& seller, uint32_t qty, double price);
    void printCancel(const Order& order);

private:
    const LogLevel d_level;
    std::ofstream d_file;
};

} // namespace orderbook
#endif // LOGGER_H
