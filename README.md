# Mini Event Backtester

A simple C++ project that implements the core building blocks of an event-driven trading backtester. It includes basic market data structures, order representations, fill processing, and position/equity tracking.

## Features

- Market data snapshots
  - Best bid
  - Best ask
  - Bid-ask spread
  - Mid price
- Order types
  - Add Order
  - Modify Order
  - Cancel Order
- Trade fill processing
- Position tracking
- Cash balance tracking
- Equity calculation
- Simulated order latency

## Project Structure

| Structure | Description |
|-----------|-------------|
| `BookLevel` | Represents one price level in the order book |
| `BookSnapshot` | Snapshot of bids and asks for one symbol |
| `Order` | Limit order information |
| `AddOrder` | Event representing a new order |
| `ModifyOrder` | Event representing an order modification |
| `CancelOrder` | Event representing an order cancellation |
| `Fill` | Executed trade information |
| `PositionState` | Tracks current position and cash |

## Utility Functions

### Market Data

- `best_bid()`
- `best_ask()`
- `spread()`
- `mid_price()`

### Trading

- `signed_quantity()`
- `apply_fill()`
- `equity()`
- `arrival_time()`

## Example

The program constructs a sample order book:

| Bid | Qty | Ask | Qty |
|-----:|----:|----:|----:|
|100.00|50|100.05|40|
|99.95|75|100.10|60|
|99.90|100|100.15|90|

It then demonstrates:

1. Computing market statistics.
2. Creating a buy order.
3. Processing fills.
4. Updating portfolio state.

## Build

Using **g++**:

```bash
g++ -std=c++20 -O2 main.cpp -o main
```

Run:

```bash
./main
```

On Windows PowerShell:

```powershell
.\main.exe
```

## Current Status

Implemented:

- ✅ Market data representation
- ✅ Order representations
- ✅ Fill processing
- ✅ Position accounting
- ✅ Cash accounting
- ✅ Equity calculation
- ✅ Latency helper

Planned Improvements:

- Event queue
- Order book simulator
- Matching engine
- Partial fills
- Multiple symbols
- Portfolio management
- PnL statistics
- CSV market data loader
- Performance metrics
- Strategy interface

## Requirements

- C++20 compatible compiler
- GCC 10+ (or equivalent)

## Author

Mini Event Backtester implemented in C++ as a foundation for event-driven trading strategy simulation.