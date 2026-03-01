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

Output is logged to `logs/orderbook.log` and the live monitoring terminal UI is started which reads from the log file.

<img width="2710" height="1546" alt="image" src="https://github.com/user-attachments/assets/158545f7-a77a-4b67-8640-6fd9cca5fe08" />

## Clean

```bash
make clean
```
