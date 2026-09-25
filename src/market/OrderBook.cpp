#include "market/OrderBook.hpp"

namespace mme {

void OrderBook::apply(const MarketEvent& event) {
    // TODO(Phase 2): Implement book updates.
    //   - Snapshot: set the level (the caller clears the book before a new
    //     snapshot starts).
    //   - Update:   quantity > 0 -> set the level; quantity == 0 -> erase it.
    //   - Trade:    does not change an L2 book directly (the exchange sends a
    //     separate Update), so ignore it here.
    (void)event;
}

void OrderBook::clear() {
    bids_.clear();
    asks_.clear();
}

bool OrderBook::empty() const {
    return bids_.empty() && asks_.empty();
}

std::size_t OrderBook::bidLevelCount() const {
    return bids_.size();
}

std::size_t OrderBook::askLevelCount() const {
    return asks_.size();
}

std::optional<Order> OrderBook::bestBid() const {
    if (bids_.empty()) {
        return std::nullopt;
    }
    const auto& [price, quantity] = *bids_.begin();
    return Order{price, quantity, Side::Buy};
}

std::optional<Order> OrderBook::bestAsk() const {
    if (asks_.empty()) {
        return std::nullopt;
    }
    const auto& [price, quantity] = *asks_.begin();
    return Order{price, quantity, Side::Sell};
}

std::vector<Order> OrderBook::topBids(std::size_t depth) const {
    std::vector<Order> levels;
    for (const auto& [price, quantity] : bids_) {
        if (levels.size() == depth) {
            break;
        }
        levels.push_back(Order{price, quantity, Side::Buy});
    }
    return levels;
}

std::vector<Order> OrderBook::topAsks(std::size_t depth) const {
    std::vector<Order> levels;
    for (const auto& [price, quantity] : asks_) {
        if (levels.size() == depth) {
            break;
        }
        levels.push_back(Order{price, quantity, Side::Sell});
    }
    return levels;
}

}  // namespace mme
