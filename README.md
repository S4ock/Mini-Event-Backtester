# Mini Event Backtester

A lightweight C++ event-driven backtesting engine for limit-order-book data.

This project replays historical order events from CSV, maintains a simulated order book, runs a strategy with configurable latency, and tracks portfolio PnL/position over time.

## What It Does

- Loads historical message data (add/modify/cancel events).
- Reconstructs and updates a live in-memory LOB (`Book`).
- Applies matching/crossing logic to generate fills.
- Calls strategy logic on each event-time step.
- Queues strategy-generated commands with latency.
- Updates a portfolio with fees, inventory, and cash.
- Prints final results and open orders.

## Repository Structure

```text
Mini-Event-Backtester/
├─ data/
│  ├─ AMZN_2012-06-21_34200000_57600000_message_10.csv
│  ├─ GOOG_2012-06-21_34200000_57600000_message_10.csv
│  └─ SPY_2012-06-21_34200000_37800000_message_50.csv
├─ include/
│  ├─ Common.hpp
│  ├─ Book.hpp
│  ├─ Portfolio.hpp
│  ├─ Strategy.hpp
│  └─ Backtest.hpp
└─ src/
   ├─ main.cpp
   ├─ Book.cpp
   ├─ Portfolio.cpp
   ├─ Strategy.cpp
   └─ Backtest.cpp
```

## Core Components

### `include/Common.hpp`
Shared domain types and constants:

- Time/ID/price aliases (`Time`, `OrderId`, `Price`, ...)
- Side enum (`Buy`, `Sell`)
- Core structs (`Order`, `Fill`, `OrderCommand`, `OrderEvent`, ...)
- Event and command enums
- Fee constant: `fee_rate = 0.0002`

### `include/Book.hpp` + `src/Book.cpp`
Order book implementation:

- Bid/ask ladders using price-keyed maps of FIFO-like order sets.
- Fast order lookup for modify/cancel by order id.
- Matching engine in `checkCross(...)`:
  - Buy crosses asks from best ask upward.
  - Sell crosses bids from best bid downward.
  - Produces `Fill` records for both resting and incoming orders.

### `include/Portfolio.hpp` + `src/Portfolio.cpp`
Portfolio and risk state:

- Tracks `cash_` (initially `100000.0`) and `position_`.
- Applies fills with transaction fee deduction.
- Tracks active internal orders and order history.

### `include/Strategy.hpp` + `src/Strategy.cpp`
Inventory-aware market-making / mean-reversion logic:

- Maintains running and short-term mid-price estimates.
- Estimates fair value and volatility-aware spread.
- Shifts quote center by inventory and recent fill flow.
- Issues add/modify/cancel commands while respecting:
  - Position limits (`kMaxPosition`)
  - Cash capacity for bids
  - Existing open order state

### `include/Backtest.hpp` + `src/Backtest.cpp`
Backtest orchestration:

- Loads CSV historical events into a priority queue.
- Replays events in timestamp order.
- Applies order commands to the book.
- Feeds fills into portfolio.
- Invokes strategy at each simulation step.
- Schedules internal strategy orders with latency.

### `src/main.cpp`
Program entrypoint configuration:

- Selects historical file.
- Defines start/end timestamps and strategy latency.
- Constructs `Book`, `Portfolio`, `Strategy`, `Backtest`.
- Executes `run()` then `printResults()`.

## Data Contract

The loader expects CSV rows with the following columns in this order:

1. Time
2. Event type
3. Order ID
4. Size
5. Price
6. Direction

Current parsing behavior in `Backtest::load_historical_data(...)`:

- Event types handled: `1` (add), `2` (modify), `3` (cancel).
- Time is converted to nanoseconds by multiplying by `1e9`.
- Price is divided by `10000.0` to convert to dollars.
- Direction `1` maps to `Buy`, otherwise `Sell`.

## Build

### Windows (MSYS2 g++)

```powershell
g++ -std=c++17 -Iinclude src/*.cpp -O2 -o backtester.exe
```

## Run

```powershell
.\backtester.exe
```

Example output shape:

```text
Started Backtest from ... to ...
Final Cash: ...
Final Position: ...
Open Orders:
Final estimated money: ...
Order ID: ..., Side: ..., Quantity: ..., Limit Price: ...
```

## How the Event Loop Works

1. Historical events are preloaded into a min-time priority queue.
2. Backtest advances simulation time to the next queued timestamp.
3. All commands at current time are applied.
4. Crosses generate fills; internal fills create strategy-visible `OrderEvent`s.
5. Strategy receives:
   - current time
   - current book
   - current portfolio
   - recent internal order events
6. Strategy commands are latency-shifted and reinserted into the queue.

## Current Defaults in `main.cpp`

- Dataset: `data/GOOG_2012-06-21_34200000_57600000_message_10.csv`
- Start time: `34200017459617`
- End time: `57599959359650`
- Strategy latency: `1000000` (nanoseconds)

## Notes and Assumptions

- The executable currently marks final inventory to the best bid in `printResults()`.
- Strategy-generated orders use `owner_id = 1`; external data defaults to `owner_id = 0`.
- Modify semantics for external events are interpreted as cancellation size reduction.

## Next Improvements (Optional)

- Add `CMakeLists.txt` for portable builds.
- Add unit tests for matching and portfolio accounting.
- Add CLI arguments for dataset/time range/latency.
- Add per-trade and per-interval performance metrics (PnL curve, drawdown, Sharpe).
