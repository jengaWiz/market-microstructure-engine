#include <gtest/gtest.h>

#include <vector>

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

TEST(OrderBookTest, BestBidIsHighestAndBestAskIsLowest) {
    OrderBook book;
    // Out of order on purpose: the map must keep each side sorted.
    book.apply(update(Side::Buy, 98, 1));
    book.apply(update(Side::Buy, 100, 2));
    book.apply(update(Side::Buy, 99, 3));
    book.apply(update(Side::Sell, 103, 4));
    book.apply(update(Side::Sell, 101, 5));
    book.apply(update(Side::Sell, 102, 6));

    EXPECT_EQ(book.bestBid()->price, 100);
    EXPECT_EQ(book.bestAsk()->price, 101);
}

TEST(OrderBookTest, RemovingBestLevelPromotesNextBest) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 2));
    book.apply(update(Side::Buy, 99, 3));
    book.apply(update(Side::Sell, 101, 5));
    book.apply(update(Side::Sell, 102, 6));

    book.apply(update(Side::Buy, 100, 0));
    book.apply(update(Side::Sell, 101, 0));

    EXPECT_EQ(book.bestBid()->price, 99);
    EXPECT_EQ(book.bestAsk()->price, 102);
}

TEST(OrderBookTest, TopLevelsAreBestFirst) {
    OrderBook book;
    book.apply(update(Side::Buy, 98, 1));
    book.apply(update(Side::Buy, 100, 2));
    book.apply(update(Side::Buy, 99, 3));
    book.apply(update(Side::Sell, 103, 4));
    book.apply(update(Side::Sell, 101, 5));
    book.apply(update(Side::Sell, 102, 6));

    const std::vector<Order> bids = book.topBids(3);
    ASSERT_EQ(bids.size(), 3u);
    EXPECT_EQ(bids[0].price, 100);
    EXPECT_EQ(bids[1].price, 99);
    EXPECT_EQ(bids[2].price, 98);
    EXPECT_EQ(bids[1].quantity, 3);

    const std::vector<Order> asks = book.topAsks(3);
    ASSERT_EQ(asks.size(), 3u);
    EXPECT_EQ(asks[0].price, 101);
    EXPECT_EQ(asks[1].price, 102);
    EXPECT_EQ(asks[2].price, 103);
    EXPECT_EQ(asks[1].quantity, 6);
}

TEST(OrderBookTest, TopLevelsRespectDepth) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 1));
    book.apply(update(Side::Buy, 99, 1));
    book.apply(update(Side::Buy, 98, 1));

    EXPECT_TRUE(book.topBids(0).empty());

    const std::vector<Order> topTwo = book.topBids(2);
    ASSERT_EQ(topTwo.size(), 2u);
    EXPECT_EQ(topTwo[0].price, 100);
    EXPECT_EQ(topTwo[1].price, 99);

    // Asking for more levels than exist returns all of them.
    EXPECT_EQ(book.topBids(10).size(), 3u);
}

TEST(OrderBookTest, ClearRemovesEveryLevel) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 5));
    book.apply(update(Side::Sell, 101, 3));

    book.clear();

    EXPECT_TRUE(book.empty());
}

}  // namespace mme
