#include "research/FeatureEngine.hpp"

namespace mme {

std::optional<double> FeatureEngine::bestBid(const OrderBook& book) const {
    const std::optional<Order> level = book.bestBid();
    if (!level) {
        return std::nullopt;
    }
    return level->price;
}

std::optional<double> FeatureEngine::bestAsk(const OrderBook& book) const {
    const std::optional<Order> level = book.bestAsk();
    if (!level) {
        return std::nullopt;
    }
    return level->price;
}

std::optional<double> FeatureEngine::midPrice(const OrderBook& book) const {
    // TODO(Phase 6): Return (bestBid + bestAsk) / 2 when both sides exist.
    (void)book;
    return std::nullopt;
}

std::optional<double> FeatureEngine::spread(const OrderBook& book) const {
    // TODO(Phase 6): Return bestAsk - bestBid when both sides exist.
    (void)book;
    return std::nullopt;
}

std::optional<double> FeatureEngine::imbalance(const OrderBook& book,
                                               std::size_t depth) const {
    // TODO(Phase 6): Sum quantities from book.topBids(depth) and
    // book.topAsks(depth), then return (bid - ask) / (bid + ask).
    // Return std::nullopt when the total quantity is zero.
    (void)book;
    (void)depth;
    return std::nullopt;
}

MarketFeatures FeatureEngine::compute(const OrderBook& book) const {
    MarketFeatures features;
    features.bestBid = bestBid(book);
    features.bestAsk = bestAsk(book);
    features.midPrice = midPrice(book);
    features.spread = spread(book);
    features.imbalance = imbalance(book);
    return features;
}

}  // namespace mme
