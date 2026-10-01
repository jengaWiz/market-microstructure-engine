#pragma once

#include <cstdint>
#include <functional>

#include "market/Order.hpp"
#include "market/Units.hpp"

namespace mme {

// The kinds of market-data messages the engine understands.
enum class EventType {
    // One price level from a full order-book snapshot. An exchange snapshot
    // (hundreds of levels) becomes a burst of Snapshot events.
    Snapshot,

    // A change to one price level. `quantity` is the NEW total quantity at
    // that price (not a delta). A quantity of 0 means the level was removed.
    Update,

    // A trade that happened at `price` for `quantity`. `side` is the side of
    // the aggressor (the order that crossed the spread).
    Trade
};

inline const char* toString(EventType type) {
    switch (type) {
        case EventType::Snapshot: return "SNAPSHOT";
        case EventType::Update:   return "UPDATE";
        case EventType::Trade:    return "TRADE";
    }
    return "UNKNOWN";
}

// A single, exchange-independent market-data event.
//
// This is the one data type that flows through the whole pipeline. Live data
// (MarketDataFeed) and recorded data (ReplayEngine) both produce MarketEvents,
// so everything downstream (OrderBook, EventRecorder, FeatureEngine) behaves
// the same whether the data is live or replayed.
//
// Exchange-specific details (JSON field names, string prices, product IDs,
// etc.) must be translated into this struct inside the feed layer and never
// leak further into the system.
//
// TODO(Phase 3): Consider adding a symbol / product ID once we handle more
// than one instrument, and an exchange sequence number to detect gaps.
struct MarketEvent {
    // Nanoseconds since the Unix epoch (1970-01-01 00:00:00 UTC).
    std::int64_t timestamp = 0;
    EventType type = EventType::Update;
    Price price = 0;        // in ticks, see Units.hpp
    Quantity quantity = 0;  // in lots,  see Units.hpp
    Side side = Side::Buy;
};

// A function that receives events. Both the live feed and the replay engine
// push events into one of these, which is how they share one pipeline.
using EventHandler = std::function<void(const MarketEvent&)>;

}  // namespace mme
