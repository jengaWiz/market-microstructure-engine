#include "market/OrderBook.hpp"

namespace mme {

void OrderBook::apply(const MarketEvent& event) {
    // TODO(Phase 2): Reject this explicitly and report it to the caller.
    if (event.quantity < 0) {
        return;
    }

    switch (event.type) {
        case EventType::Update:
        case EventType::Snapshot:
            // A snapshot level and an update both mean "the total at this
            // price is now `quantity`". The caller has already cleared the
            // book if this snapshot replaces an old one.
            setLevel(event.side, event.price, event.quantity);
            break;
        case EventType::Trade:
            // The exchange sends a separate Update for any level a trade
            // changed, so applying the trade here would double-count it.
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
