# Mini Event Backtester

Mini Event Backtester is a compact C++ project demonstrating the core building blocks of an event-driven trading backtester. It is intended as a small, easy-to-read foundation for learning how market data, orders, fills, and portfolio accounting interact in a backtesting environment.

This README describes the repository layout, how to build and run the example, and where to look in the code to extend the project.

## Key Features

- Simple order book representation (bid / ask levels)
- Limit order events: add, modify, cancel
- Trade fill processing and position/cash accounting
- Equity calculation and basic latency simulation
- Small, self-contained C++ codebase intended for education and experimentation

## Repository layout

- `include/`
  - `Backtest.hpp`  — Core backtest types and helpers
  - `Book.hpp`      — Order book and market snapshot types
  - `Common.hpp`    — Common utilities and type aliases
  - `Portfolio.hpp` — Position and cash accounting (equity calculation)
  - `Strategy.hpp`  — Simple strategy interface / helpers

- `src/`
  - `Book.cpp`      — Implementation of order book and market helpers
  - `Portfolio.cpp` — Portfolio / position accounting implementation
  - `Strategy.cpp`  — Example strategy logic
  - `main.cpp`      — Small example that wires components together and runs a demo

- `.git/` and other project files

Files above are small and organized so that each component (book, portfolio, strategy) is easy to read and extend.

## Build

A C++20-compatible compiler is required.

Linux / macOS (g++/clang):

```bash
g++ -std=c++20 -O2 -Iinclude src/*.cpp -o backtester
```

Windows (MSVC via Developer Command Prompt):

```powershell
cl /std:c++20 /O2 /Iinclude src\*.cpp /Fe:backtester.exe
```

Windows (MinGW / g++):

```powershell
g++ -std=c++20 -O2 -Iinclude src\*.cpp -o backtester.exe
```

## Run

From the repository root after building:

Linux/macOS:

```bash
./backtester
```

Windows PowerShell:

```powershell
.\backtester.exe
```

The example program runs a tiny scenario demonstrating market snapshots, order creation, fills, and portfolio updates. Output is printed to stdout.

## Where to look to extend functionality

- To extend market handling, order matching, or a more complete matching engine, start in `src/Book.cpp` and `include/Book.hpp`.
- To implement event queuing or richer event-driven flows, add an event queue manager and drive events from `main.cpp` or a new `Engine` class.
- To wire strategy code into the loop for automated testing, extend `include/Strategy.hpp` and `src/Strategy.cpp`.
- To add CSV playback of market data, create a small loader and feed book snapshots into the event loop.

## Ideas & Planned Improvements

- Full event queue with timestamps and ordering
- Matching engine supporting partial fills and order matching by price/time
- Support for multiple instruments and portfolio-level risk checks
- CSV/Feeder for historical data playback
- PnL/statistics reporting and performance metrics

## Requirements

- C++20 compiler (g++ 10+/Clang/MSVC with C++20 support)

## Contributing

This repository is small and educational. Suggestions, small fixes, and PRs that improve documentation or add simple, well-documented functionality are welcome.

When contributing:
- Keep changes small and focused
- Add tests or examples for non-trivial features
- Update this README when adding new top-level features

## License

This project does not include a license file. Add a LICENSE file to specify reuse terms (e.g., MIT, Apache-2.0) if you plan to share the code publicly.

## Contact

For questions about the code layout or suggestions for improvements, leave an issue or contact the repository owner.
