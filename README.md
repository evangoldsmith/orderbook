# Orderbook

A limit order book and matching engine written in C++20. Supports price-time priority (FIFO) and pro-rata matching algorithms. Orders are matched against resting orders in the book, with partial fills and remainder handling. Includes a logger that writes order and trade events to `logs/orderbook.log`.

## Building

```bash
make build
```

## Running Tests

```bash
make test
```

## Demo

Runs a simulation that sends random buy/sell orders into the book and logs all activity.

```bash
make demo
```

Output is printed to the console and logged to `logs/orderbook.log`.

## Clean

```bash
make clean
```
