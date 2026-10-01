#include "market/OrderBook.hpp"

namespace mme {

void OrderBook::apply(const MarketEvent& event) {
    // TODO(Phase 2): Reject this explicitly and report it to the caller.
    if (event.quantity < 0) {
        return;
    }

    switch (event.type) {
        case EventType::Update:
            setLevel(event.side, event.price, event.quantity);
            break;
        case EventType::Snapshot:
            // TODO(Phase 2): Set the level (the caller clears the book before
            // a new snapshot starts).
            break;
        case EventType::Trade:
            // TODO(Phase 2): Trades don't change an L2 book directly (the
            // exchange sends a separate Update), so they will be ignored.
            break;
    }
}

void OrderBook::setLevel(Side side, Price price, Quantity quantity) {
    // The quantity is the new total, so we overwrite instead of adding.
    // std::map keeps each side sorted, so the best price stays first.
    if (side == Side::Buy) {
        if (quantity == 0) {
            bids_.erase(price);
        } else {
            bids_[price] = quantity;
        }
    } else {
        if (quantity == 0) {
            asks_.erase(price);
        } else {
            asks_[price] = quantity;
        }
    }
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
