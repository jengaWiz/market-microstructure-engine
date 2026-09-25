#include <gtest/gtest.h>

#include "market/OrderBook.hpp"

namespace mme {

TEST(OrderBookTest, NewBookIsEmpty) {
    const OrderBook book;

    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.bidLevelCount(), 0u);
    EXPECT_EQ(book.askLevelCount(), 0u);
}

TEST(OrderBookTest, EmptyBookHasNoBestPrices) {
    const OrderBook book;

    EXPECT_FALSE(book.bestBid().has_value());
    EXPECT_FALSE(book.bestAsk().has_value());
}

TEST(OrderBookTest, EmptyBookHasNoDepth) {
    const OrderBook book;

    EXPECT_TRUE(book.topBids(5).empty());
    EXPECT_TRUE(book.topAsks(5).empty());
}

// TODO(Phase 2): Add tests for apply(): inserting levels, updating levels,
// removing levels with quantity 0, and best bid / best ask ordering.

}  // namespace mme
