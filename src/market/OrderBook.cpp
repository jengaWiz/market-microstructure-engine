#include "market/OrderBook.hpp"

namespace mme {

bool OrderBook::apply(const MarketEvent& event) {
    if (!isValid(event)) {
        return false;
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
    return true;
}

bool OrderBook::isValid(const MarketEvent& event) {
    if (event.price <= 0 || event.quantity < 0) {
        return false;
    }
    // A quantity of 0 means "remove this level" for book events, but a trade
    // of nothing makes no sense.
    if (event.type == EventType::Trade && event.quantity == 0) {
        return false;
    }
    return true;
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

bool OrderBook::isCrossed() const {
    if (bids_.empty() || asks_.empty()) {
        return false;
    }
    return bids_.begin()->first >= asks_.begin()->first;
}

}  // namespace mme
