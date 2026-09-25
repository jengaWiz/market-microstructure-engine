#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <vector>

#include "market/MarketEvent.hpp"
#include "market/Order.hpp"

namespace mme {

// A local Level 2 order book for a single instrument.
//
// Each side is a sorted map of price -> total quantity at that price:
//   - bids are sorted highest price first (the best bid is the highest)
//   - asks are sorted lowest price first  (the best ask is the lowest)
//
// The book is deterministic: the same sequence of events always produces
// the same book state. It does no I/O and no threading.
class OrderBook {
public:
    // Applies one market event to the book.
    // TODO(Phase 2): Not implemented yet; currently does nothing.
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
    // TODO(Phase 2): Using double as a map key is fine for learning but
    // fragile in general. Revisit together with integer price ticks.
    std::map<double, double, std::greater<double>> bids_;  // price -> quantity
    std::map<double, double> asks_;                        // price -> quantity
};

}  // namespace mme
