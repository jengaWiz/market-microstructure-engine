#include <gtest/gtest.h>

#include "market/OrderBook.hpp"
#include "market/Units.hpp"
#include "research/FeatureEngine.hpp"

namespace mme {

TEST(FeatureEngineTest, EmptyBookProducesNoFeatures) {
    const OrderBook book;
    const FeatureEngine engine(TickSize{0.01});

    const MarketFeatures features = engine.compute(book);

    EXPECT_FALSE(features.bestBid.has_value());
    EXPECT_FALSE(features.bestAsk.has_value());
    EXPECT_FALSE(features.midPrice.has_value());
    EXPECT_FALSE(features.spread.has_value());
    EXPECT_FALSE(features.imbalance.has_value());
}

// TODO(Phase 6): Once OrderBook::apply() works (Phase 2), build small books
// by hand and check mid-price, spread, and imbalance against known values.

}  // namespace mme
