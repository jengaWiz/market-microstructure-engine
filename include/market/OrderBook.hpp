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
    // TODO(Phase 2): Handle Snapshot and Trade events (they are ignored for
    // now), and reject invalid events such as a negative quantity.
    void apply(const MarketEvent& event);

    // Removes every price level from both sides.
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
    // Sets one price level on the given side, or removes it if `quantity`
    // is 0.
    void setLevel(Side side, Price price, Quantity quantity);

    std::map<Price, Quantity, std::greater<Price>> bids_;  // price -> quantity
    std::map<Price, Quantity> asks_;                       // price -> quantity
};

}  // namespace mme
