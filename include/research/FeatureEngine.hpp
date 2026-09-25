#pragma once

#include <cstddef>
#include <optional>

#include "market/OrderBook.hpp"

namespace mme {

// All features computed from one order-book state.
// A feature is std::nullopt when it cannot be computed (e.g. an empty book).
struct MarketFeatures {
    std::optional<double> bestBid;
    std::optional<double> bestAsk;
    std::optional<double> midPrice;
    std::optional<double> spread;
    std::optional<double> imbalance;
};

// Calculates simple market features from an OrderBook.
//
// The FeatureEngine only reads the book; it never changes it. Every method is
// a pure calculation, so the same book always gives the same features.
class FeatureEngine {
public:
    // Price of the highest bid / lowest ask.
    std::optional<double> bestBid(const OrderBook& book) const;
    std::optional<double> bestAsk(const OrderBook& book) const;

    // (bestBid + bestAsk) / 2
    // TODO(Phase 6): Not implemented; returns std::nullopt.
    std::optional<double> midPrice(const OrderBook& book) const;

    // bestAsk - bestBid
    // TODO(Phase 6): Not implemented; returns std::nullopt.
    std::optional<double> spread(const OrderBook& book) const;

    // Order-book imbalance over the top `depth` levels of each side:
    //   (bidQuantity - askQuantity) / (bidQuantity + askQuantity)
    // Ranges from -1 (only asks) to +1 (only bids).
    // TODO(Phase 6): Not implemented; returns std::nullopt.
    std::optional<double> imbalance(const OrderBook& book,
                                    std::size_t depth = 1) const;

    // Computes every feature above in one call.
    MarketFeatures compute(const OrderBook& book) const;
};

}  // namespace mme
