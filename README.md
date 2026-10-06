# Market Microstructure Research Engine

[![CI](https://github.com/jengaWiz/market-microstructure-engine/actions/workflows/ci.yml/badge.svg)](https://github.com/jengaWiz/market-microstructure-engine/actions/workflows/ci.yml)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)
![CMake](https://img.shields.io/badge/build-CMake-064F8C.svg)
![Status](https://img.shields.io/badge/status-Phase%201%20of%208-orange.svg)

A small, beginner-friendly C++20 project for studying how crypto markets behave at the level of individual order-book changes.

This is a research and learning tool. It does **not** trade, submit orders, or run strategies.

---

## Contents

- [Intended final product](#intended-final-product)
- [Current progress](#current-progress)
- [Roadmap](#roadmap)
- [Background: what is a limit order book?](#background-what-is-a-limit-order-book)
- [Research question](#research-question)
- [Project layout](#project-layout)
- [Building](#building)
- [Running tests](#running-tests)
- [Dependencies](#dependencies)

---

## Intended final product

When finished, the engine will be a single-threaded C++ pipeline that runs in two interchangeable modes:

| Mode       | Event source                                   | Output                                        |
|------------|------------------------------------------------|-----------------------------------------------|
| **Live**   | A crypto exchange's L2 WebSocket feed          | Recorded events (`data/raw/`) + features      |
| **Replay** | A recording made earlier in live mode          | Features (`data/processed/`), reproducible    |

It will:

1. **Connect** to a live crypto exchange market-data feed (e.g. `BTC-USD`).
2. **Maintain** a local Level 2 order book from snapshots and incremental updates.
3. **Record** every normalized market event to disk as CSV, building a private historical dataset.
4. **Replay** recorded events offline, deterministically, through exactly the same code path as live data.
5. **Compute** features on every book change: best bid/ask, mid-price, spread, and order-book imbalance.
6. **Analyze** whether those features predict short-term mid-price movement (see [Research question](#research-question)).
7. **Profile and optimize** the hot path once correctness is established.

```
  live:    Exchange  ──► MarketDataFeed ──┐
                                          ├──► MarketEvent ──► OrderBook ──► FeatureEngine ──► data/processed/
  replay:  data/raw/ ──► ReplayEngine  ───┘          │
                                                     └──► EventRecorder ──► data/raw/   (live mode only)
```

The full design, and the reasoning behind it, is in [docs/architecture.md](docs/architecture.md).

## Current progress

**Phase 1 of 8 is complete: project structure and domain models.**

| Component        | State |
|------------------|-------|
| Build system     | ✅ CMake project, strict warnings, CI on Linux and macOS |
| `Order`, `MarketEvent` | ✅ Domain types defined and tested |
| `OrderBook`      | 🟡 Data structures and read-only queries (`bestBid`, `bestAsk`, `topBids`, ...) work; `apply()` is a stub |
| `FeatureEngine`  | 🟡 `bestBid` / `bestAsk` work; `midPrice`, `spread`, `imbalance` are stubs |
| `MarketDataFeed` | ⬜ Interface only, no network code |
| `EventRecorder`  | ⬜ Interface only, no file I/O |
| `ReplayEngine`   | ⬜ Interface and `run()` loop only; `load()` is a stub |
| Research analysis | ⬜ Not started |

Every component already has its final header interface and a placeholder implementation, so the whole pipeline compiles and runs end to end today. Search for `TODO(Phase N)` to see exactly where each remaining piece goes.

Running the app today prints:

```
Market Microstructure Research Engine
Status: Phase 1 (project skeleton)

Live feed for BTC-USD is not implemented yet (Phase 3).
Order book is empty.
```

## Roadmap

| Phase | Goal                                   | Status      |
|-------|----------------------------------------|-------------|
| 1     | Project structure and domain models    | ✅ Done     |
| 2     | Order book implementation              | ⏭️ Next     |
| 3     | Live market-data connection            |             |
| 4     | Event recording                        |             |
| 5     | Historical replay                      |             |
| 6     | Feature calculation                    |             |
| 7     | Research and statistical analysis      |             |
| 8     | Performance profiling and optimization |             |

## Background: what is a limit order book?

An exchange keeps two lists of people waiting to trade:

- **Bids**: offers to *buy*, e.g. "I'll buy 2 BTC at $60,000."
- **Asks**: offers to *sell*, e.g. "I'll sell 1 BTC at $60,010."

Together these lists form the **limit order book**. Bids are sorted from the highest price down, and asks from the lowest price up.

- The **best bid** is the highest price anyone will pay right now.
- The **best ask** is the lowest price anyone will sell for right now.
- The **spread** is the gap between them (`best ask - best bid`).
- The **mid-price** is halfway between them, a common estimate of the "current price."

A trade happens when someone is willing to cross the spread, for example by buying at the best ask.

A **Level 2 (L2)** feed shows the total quantity waiting at each price, but not the individual orders behind it. For example, it tells you there are 5.2 BTC bid at $60,000, but not who placed them.

**Order-book imbalance** compares how much quantity is waiting on the bid side against the ask side near the top of the book:

```
imbalance = (bid quantity - ask quantity) / (bid quantity + ask quantity)
```

It ranges from -1 (all asks) to +1 (all bids).

## Research question

> Does order-book imbalance contain information about the direction of the next short-term price movement?

Put simply: when there is noticeably more buying interest than selling interest at the top of the book, is the mid-price more likely to tick up next?

## Project layout

```
include/        Public headers, one folder per component
  market/       Units, Order, MarketEvent, OrderBook
  feed/         MarketDataFeed   (live exchange -> MarketEvent)
  recorder/     EventRecorder    (MarketEvent -> disk)
  replay/       ReplayEngine     (disk -> MarketEvent)
  research/     FeatureEngine    (OrderBook -> features)
src/            Implementations, mirroring include/, plus main.cpp
tests/          GoogleTest unit tests
data/raw/       Recorded market events (not committed to git)
data/processed/ Computed features (not committed to git)
docs/           Design notes; start with docs/architecture.md
```

## Building

Requirements:

- a C++20 compiler (Clang 14+, GCC 11+, or MSVC 2022)
- CMake 3.20 or newer
- an internet connection the first time you configure, so CMake can download GoogleTest

```bash
cmake -S . -B build
cmake --build build
```

Run the application:

```bash
./build/mme
```

## Running tests

```bash
ctest --test-dir build --output-on-failure
```

Or run the test binary directly, which gives more detailed GoogleTest output:

```bash
./build/mme_tests
```

To build without tests (and skip the GoogleTest download), configure with `-DMME_BUILD_TESTS=OFF`.

## Dependencies

- **GoogleTest** v1.15.2 for tests only. CMake downloads it automatically with `FetchContent`, so you don't need to install anything.

The engine itself uses only the C++ standard library. A WebSocket/JSON library will be chosen in Phase 3.
