#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "market/MarketEvent.hpp"
#include "market/Order.hpp"
#include "market/Units.hpp"

namespace mme {

// A local Level 2 order book for a single instrument.
//
// Each side is a sorted map of price -> total quantity at that price, both
// stored as whole numbers (ticks and lots, see Units.hpp):
//   - bids are sorted highest price first (the best bid is the highest)
//   - asks are sorted lowest price first  (the best ask is the lowest)
//
// The book is deterministic: the same sequence of events always produces
// the same book state. It does no I/O and no threading.
class OrderBook {
public:
    // Applies one market event to the book.
    //
    // Update: `quantity` is the NEW total at `price`, not a change.
    //   - quantity > 0  -> the level is set to that quantity (added if new,
    //                      overwritten if it already exists)
    //   - quantity == 0 -> the level is removed (a no-op if it isn't there)
    //
    // Snapshot: one level from a full copy of the exchange's book. It is set
    //   exactly like an Update. The book can't tell where one snapshot ends
    //   and the next begins, so the CALLER must call clear() before applying
    //   a new snapshot. Otherwise levels missing from the new snapshot would
    //   stay in the book.
    //
    // Trade: leaves the book unchanged. When a trade uses up quantity at a
    //   level, the exchange also sends an Update with that level's new total.
    //   Changing the book on the trade too would count the change twice.
    //   Trades still matter for research, just not for the book.
    //
    // Returns true if the event was valid, false if it was rejected. A
    // rejected event leaves the book unchanged. An event is invalid if:
    //   - its price is 0 or below
    //   - its quantity is below 0
    //   - it is a Trade with a quantity of 0 (nothing was traded)
    // A rejected event usually means corrupt or misread data, so the caller
    // should treat the book as unreliable and resync from a fresh snapshot.
    bool apply(const MarketEvent& event);

    // Removes every price level from both sides.
    //
    // Call this before applying a new snapshot: a snapshot replaces the whole
    // book rather than changing it. In live mode the MarketDataFeed does this
    // automatically whenever a snapshot message arrives (Phase 3).
    void clear();

    // True when the book has no bids and no asks.
    bool empty() const;

    // Number of distinct price levels on each side.
    std::size_t bidLevelCount() const;
    std::size_t askLevelCount() const;

    // Highest bid / lowest ask, or std::nullopt if that side is empty.
    std::optional<Order> bestBid() const;
    std::optional<Order> bestAsk() const;

    // Up to `depth` best levels on each side, best price first.
    // Used by the FeatureEngine for depth-based features such as imbalance.
    std::vector<Order> topBids(std::size_t depth) const;
    std::vector<Order> topAsks(std::size_t depth) const;

private:
    // True if `event` passes the checks described on apply().
    static bool isValid(const MarketEvent& event);

    // Sets one price level on the given side, or removes it if `quantity`
    // is 0.
    void setLevel(Side side, Price price, Quantity quantity);

    std::map<Price, Quantity, std::greater<Price>> bids_;  // price -> quantity
    std::map<Price, Quantity> asks_;                       // price -> quantity
};

}  // namespace mme
