# Architecture

This document describes how the engine is organized and why. It covers the target design; see the README for which parts exist today.

## Components

| Component        | Location                  | Responsibility |
|------------------|---------------------------|----------------|
| `Order`          | `include/market/`         | One price level: price, quantity, side. |
| `MarketEvent`    | `include/market/`         | One exchange-independent event: snapshot level, update, or trade. The single data type that flows through the pipeline. |
| `OrderBook`      | `include/market/`         | Maintains bids and asks for one instrument by applying `MarketEvent`s. Answers "what is the best bid / best ask / top N levels?" |
| `MarketDataFeed` | `include/feed/`           | Connects to an exchange WebSocket and translates exchange messages into `MarketEvent`s. |
| `EventRecorder`  | `include/recorder/`       | Writes `MarketEvent`s to disk to build our own historical dataset. |
| `ReplayEngine`   | `include/replay/`         | Reads recorded events and replays them in timestamp order. |
| `FeatureEngine`  | `include/research/`       | Reads an `OrderBook` and calculates best bid, best ask, mid-price, spread, and imbalance. Never modifies the book. |

Each component is a small class with one job. Components don't know about each other: they communicate only through `MarketEvent`, `OrderBook`, and a plain callback type (`EventHandler`). This lets each one be implemented and tested on its own.

## Data flow

### Live mode

```
Live Exchange
      │  (exchange-specific JSON over WebSocket)
      ▼
MarketDataFeed
      │  MarketEvent
      ▼
  OrderBook ──────────────┐
      │                   │
      ▼                   ▼
EventRecorder       FeatureEngine
 (data/raw/)              │
                          ▼
                   Research Output
                  (data/processed/)
```

### Replay mode

```
Recorded Data (data/raw/)
      │
      ▼
ReplayEngine
      │  MarketEvent
      ▼
  OrderBook
      │
      ▼
FeatureEngine ──► Research Output
```

### How the wiring works

`MarketDataFeed` and `ReplayEngine` both accept an `EventHandler`, which is just `std::function<void(const MarketEvent&)>`. The application (`main.cpp`) supplies one handler that does the processing:

```cpp
auto handler = [&](const MarketEvent& event) {
    book.apply(event);
    recorder.record(event);        // live mode only
    features.compute(book);
};
```

Switching between live and replay mode means handing this same handler to a different event source. Nothing downstream changes.

Everything runs on a single thread and processes one event at a time. Threads will only be considered in Phase 8, and only if profiling shows a need.

## Why live and replay share one event model

1. **The same code is tested both ways.** If replay used a different format or code path, a bug could exist in live mode but not in replay (or the reverse). Sharing `MarketEvent` means the book and feature code we study offline are exactly the code that runs live.
2. **Research is reproducible.** Live data arrives once and is gone. Recording normalized `MarketEvent`s lets us rerun an analysis on the same data as often as we like and get the same answer.
3. **The recorder and replayer stay simple.** They read and write one small, stable format instead of every exchange's raw messages.

## Deterministic parts

These parts must produce the same output for the same input every time. They do no I/O, don't read the clock, and use no randomness:

- `OrderBook`: the same event sequence always gives the same book.
- `FeatureEngine`: pure functions of the book state.
- `ReplayEngine` ordering: events are sorted by timestamp with a *stable* sort, so events with equal timestamps keep their recorded order.

These parts are **not** deterministic because they depend on the outside world:

- `MarketDataFeed`: network timing, disconnects, and whatever the exchange sends.
- `EventRecorder`: disk I/O can fail.

Keeping the deterministic core separate from I/O is what makes the core easy to unit-test.

## Where exchange-specific logic lives

All exchange-specific code stays inside `src/feed/`. That includes:

- WebSocket URLs and subscription messages
- JSON parsing and field names
- converting string prices and ISO timestamps into `double` and nanoseconds
- turning one snapshot message into many `Snapshot` events
- handling sequence numbers, gaps, and resubscribing

Nothing outside `feed/` should ever mention a specific exchange. If we support a second exchange later, it gets its own translation code in `feed/`, and every other component stays unchanged.

## Deliberate simplifications (for now)

- Prices and quantities are `double`. Integer price ticks may replace them once the order book is implemented (see the TODO in `Order.hpp`).
- One instrument per `OrderBook`, with no symbol field on `MarketEvent` yet.
- The recording format will be a simple CSV, which is human-readable and easy to load in Python or a spreadsheet for Phase 7.
- No external dependencies besides GoogleTest (tests only).
