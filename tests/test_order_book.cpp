#include <gtest/gtest.h>

#include <vector>

#include "market/MarketEvent.hpp"
#include "market/OrderBook.hpp"
#include "market/Units.hpp"

namespace mme {

namespace {

// Builds an Update event. Prices are in ticks and quantities in lots, but the
// tests use small numbers so they're easy to read.
MarketEvent update(Side side, Price price, Quantity quantity) {
    return MarketEvent{0, EventType::Update, price, quantity, side};
}

MarketEvent snapshot(Side side, Price price, Quantity quantity) {
    return MarketEvent{0, EventType::Snapshot, price, quantity, side};
}

MarketEvent trade(Side aggressor, Price price, Quantity quantity) {
    return MarketEvent{0, EventType::Trade, price, quantity, aggressor};
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

TEST(OrderBookTest, SnapshotBuildsBook) {
    OrderBook book;

    book.apply(snapshot(Side::Buy, 100, 2));
    book.apply(snapshot(Side::Buy, 99, 4));
    book.apply(snapshot(Side::Sell, 101, 1));
    book.apply(snapshot(Side::Sell, 102, 3));

    EXPECT_EQ(book.bidLevelCount(), 2u);
    EXPECT_EQ(book.askLevelCount(), 2u);
    EXPECT_EQ(book.bestBid()->price, 100);
    EXPECT_EQ(book.bestBid()->quantity, 2);
    EXPECT_EQ(book.bestAsk()->price, 101);
    EXPECT_EQ(book.bestAsk()->quantity, 1);
}

TEST(OrderBookTest, UpdatesApplyOnTopOfSnapshot) {
    OrderBook book;
    book.apply(snapshot(Side::Buy, 100, 2));
    book.apply(snapshot(Side::Sell, 101, 1));

    book.apply(update(Side::Buy, 100, 7));
    book.apply(update(Side::Sell, 101, 0));

    EXPECT_EQ(book.bestBid()->quantity, 7);
    EXPECT_FALSE(book.bestAsk().has_value());
}

TEST(OrderBookTest, ClearThenSnapshotRemovesStaleLevels) {
    OrderBook book;
    book.apply(snapshot(Side::Buy, 100, 2));
    book.apply(snapshot(Side::Buy, 98, 5));  // gone by the next snapshot
    book.apply(snapshot(Side::Sell, 101, 1));

    // A new snapshot replaces the whole book, so the caller clears first.
    book.clear();
    book.apply(snapshot(Side::Buy, 100, 3));
    book.apply(snapshot(Side::Sell, 101, 1));

    // Only the new snapshot's bid is left; the stale level at 98 is gone.
    const std::vector<Order> bids = book.topBids(10);
    ASSERT_EQ(bids.size(), 1u);
    EXPECT_EQ(bids[0].price, 100);
    EXPECT_EQ(bids[0].quantity, 3);
}

TEST(OrderBookTest, TradeDoesNotChangeBook) {
    OrderBook book;
    book.apply(snapshot(Side::Buy, 100, 2));
    book.apply(snapshot(Side::Sell, 101, 3));

    // A buyer takes 1 lot at the best ask. The book must not change: the
    // exchange will send its own Update for the ask at 101.
    book.apply(trade(Side::Buy, 101, 1));
    // A trade at a price with no level must not create one either.
    book.apply(trade(Side::Sell, 95, 1));

    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.askLevelCount(), 1u);
    EXPECT_EQ(book.bestBid()->quantity, 2);
    EXPECT_EQ(book.bestAsk()->quantity, 3);
}

TEST(OrderBookTest, MixedEventTimelineEndsInExpectedBook) {
    // A short, realistic BTC-USD sequence using real prices and sizes,
    // converted to ticks and lots the way the feed will do it.
    const TickSize tick{0.01};
    const LotSize lot{0.00000001};
    auto px = [&](double dollars) { return tick.toUnits(dollars); };
    auto qty = [&](double btc) { return lot.toUnits(btc); };

    OrderBook book;
    // 1. Connect: snapshot of the exchange's book.
    book.apply(snapshot(Side::Buy, px(60000.00), qty(2)));
    book.apply(snapshot(Side::Sell, px(60010.00), qty(1)));
    // 2. More sellers join at 60,010.
    book.apply(update(Side::Sell, px(60010.00), qty(3)));
    // 3. A buyer takes 1 BTC at the best ask (book unchanged)...
    book.apply(trade(Side::Buy, px(60010.00), qty(1)));
    // 4. ...and the exchange confirms the ask's new total.
    book.apply(update(Side::Sell, px(60010.00), qty(2)));
    // 5. A new, higher bid becomes the best bid.
    book.apply(update(Side::Buy, px(60005.00), qty(4)));

    // Expected: ASKS 60,010 x 2 | BIDS 60,005 x 4, 60,000 x 2
    const std::vector<Order> asks = book.topAsks(10);
    ASSERT_EQ(asks.size(), 1u);
    EXPECT_EQ(asks[0].price, px(60010.00));
    EXPECT_EQ(asks[0].quantity, qty(2));

    const std::vector<Order> bids = book.topBids(10);
    ASSERT_EQ(bids.size(), 2u);
    EXPECT_EQ(bids[0].price, px(60005.00));
    EXPECT_EQ(bids[0].quantity, qty(4));
    EXPECT_EQ(bids[1].price, px(60000.00));
    EXPECT_EQ(bids[1].quantity, qty(2));
}

TEST(OrderBookTest, ValidEventsAreAccepted) {
    OrderBook book;

    EXPECT_TRUE(book.apply(snapshot(Side::Buy, 100, 2)));
    EXPECT_TRUE(book.apply(update(Side::Sell, 101, 3)));
    EXPECT_TRUE(book.apply(update(Side::Sell, 101, 0)));  // removal is valid
    EXPECT_TRUE(book.apply(trade(Side::Buy, 101, 1)));
}

TEST(OrderBookTest, RejectsNonPositivePrice) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 2));

    EXPECT_FALSE(book.apply(update(Side::Buy, 0, 5)));
    EXPECT_FALSE(book.apply(update(Side::Sell, -1, 5)));
    EXPECT_FALSE(book.apply(snapshot(Side::Buy, 0, 5)));
    EXPECT_FALSE(book.apply(trade(Side::Buy, -100, 1)));

    // The book is exactly as it was.
    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.askLevelCount(), 0u);
    EXPECT_EQ(book.bestBid()->price, 100);
}

TEST(OrderBookTest, RejectsNegativeQuantity) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 2));

    EXPECT_FALSE(book.apply(update(Side::Buy, 100, -1)));
    EXPECT_FALSE(book.apply(snapshot(Side::Sell, 101, -5)));
    EXPECT_FALSE(book.apply(trade(Side::Buy, 100, -1)));

    EXPECT_EQ(book.bidLevelCount(), 1u);
    EXPECT_EQ(book.bestBid()->quantity, 2);  // not changed or removed
    EXPECT_EQ(book.askLevelCount(), 0u);
}

TEST(OrderBookTest, RejectsZeroQuantityTrade) {
    OrderBook book;

    EXPECT_FALSE(book.apply(trade(Side::Buy, 100, 0)));
    EXPECT_TRUE(book.empty());
}

TEST(OrderBookTest, EmptyOrOneSidedBookIsNotCrossed) {
    OrderBook book;
    EXPECT_FALSE(book.isCrossed());

    book.apply(update(Side::Buy, 100, 1));
    EXPECT_FALSE(book.isCrossed());  // bids only

    book.clear();
    book.apply(update(Side::Sell, 100, 1));
    EXPECT_FALSE(book.isCrossed());  // asks only
}

TEST(OrderBookTest, NormalBookIsNotCrossed) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 1));
    book.apply(update(Side::Sell, 101, 1));

    EXPECT_FALSE(book.isCrossed());
}

TEST(OrderBookTest, BidEqualToAskIsCrossed) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 1));
    book.apply(update(Side::Sell, 100, 1));

    // A buyer and a seller at the same price would have traded.
    EXPECT_TRUE(book.isCrossed());
}

TEST(OrderBookTest, BidAboveAskIsCrossed) {
    OrderBook book;
    book.apply(update(Side::Buy, 102, 1));
    book.apply(update(Side::Sell, 101, 1));

    EXPECT_TRUE(book.isCrossed());

    // Once the stale level is removed, the book is consistent again.
    book.apply(update(Side::Buy, 102, 0));
    EXPECT_FALSE(book.isCrossed());
}

TEST(OrderBookTest, ClearRemovesEveryLevel) {
    OrderBook book;
    book.apply(update(Side::Buy, 100, 5));
    book.apply(update(Side::Sell, 101, 3));

    book.clear();

    EXPECT_TRUE(book.empty());
}

}  // namespace mme
