#include <gtest/gtest.h>

#include "market/MarketEvent.hpp"
#include "market/OrderBook.hpp"

namespace mme {

namespace {

// Builds an Update event. Prices are in ticks and quantities in lots, but the
// tests use small numbers so they're easy to read.
MarketEvent update(Side side, Price price, Quantity quantity) {
    return MarketEvent{0, EventType::Update, price, quantity, side};
}

}  // namespace

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

TEST(OrderBookTest, UpdateAddsBidLevel) {
    OrderBook book;

    book.apply(update(Side::Buy, 100, 5));

    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.askLevelCount(), 0u);
    ASSERT_TRUE(book.bestBid().has_value());
    EXPECT_EQ(book.bestBid()->price, 100);
    EXPECT_EQ(book.bestBid()->quantity, 5);
    EXPECT_EQ(book.bestBid()->side, Side::Buy);
}

TEST(OrderBookTest, UpdateAddsAskLevel) {
    OrderBook book;

    book.apply(update(Side::Sell, 101, 3));

    EXPECT_EQ(book.askLevelCount(), 1u);
    EXPECT_EQ(book.bidLevelCount(), 0u);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(book.bestAsk()->price, 101);
    EXPECT_EQ(book.bestAsk()->quantity, 3);
    EXPECT_EQ(book.bestAsk()->side, Side::Sell);
}

TEST(OrderBookTest, UpdateOverwritesQuantityInsteadOfAdding) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 5));

    // The quantity is the new total at that price, not a change.
    book.apply(update(Side::Buy, 100, 2));

    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.bestBid()->quantity, 2);
}

TEST(OrderBookTest, UpdateWithZeroQuantityRemovesLevel) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 5));
    book.apply(update(Side::Sell, 101, 3));

    book.apply(update(Side::Buy, 100, 0));

    EXPECT_EQ(book.bidLevelCount(), 0u);
    EXPECT_FALSE(book.bestBid().has_value());
    EXPECT_EQ(book.askLevelCount(), 1u);  // the other side is untouched
}

TEST(OrderBookTest, RemovingMissingLevelDoesNothing) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 5));

    book.apply(update(Side::Buy, 99, 0));   // no level at 99
    book.apply(update(Side::Sell, 100, 0)); // 100 exists, but on the bid side

    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.bestBid()->quantity, 5);
    EXPECT_EQ(book.askLevelCount(), 0u);
}

TEST(OrderBookTest, ClearRemovesEveryLevel) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 5));
    book.apply(update(Side::Sell, 101, 3));

    book.clear();

    EXPECT_TRUE(book.empty());
}

}  // namespace mme
