// Entry point. Wires the components together into the intended pipeline:
//
//   MarketDataFeed -> MarketEvent -> OrderBook -> EventRecorder
//                                             \-> FeatureEngine
//
// The live feed isn't implemented yet (Phase 3), so this runs a short demo:
// a hand-made BTC-USD event sequence goes through the same event handler the
// feed will use, and the resulting order book is printed.

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>

#include "feed/MarketDataFeed.hpp"
#include "market/MarketEvent.hpp"
#include "market/Order.hpp"
#include "market/OrderBook.hpp"
#include "market/Units.hpp"
#include "recorder/EventRecorder.hpp"
#include "research/FeatureEngine.hpp"

namespace {

// Prints the top `depth` levels of each side, asks above bids, the way order
// books are usually drawn: the best prices meet in the middle.
void printBook(const mme::OrderBook& book, const mme::TickSize& tick,
               const mme::LotSize& lot, std::size_t depth) {
    const std::vector<mme::Order> asks = book.topAsks(depth);
    const std::vector<mme::Order> bids = book.topBids(depth);

    std::cout << std::fixed;
    // Asks are stored lowest first; print them highest first.
    for (auto it = asks.rbegin(); it != asks.rend(); ++it) {
        std::cout << "  ASK  $" << std::setprecision(2) << tick.toReal(it->price)
                  << "  " << std::setprecision(8) << lot.toReal(it->quantity)
                  << " BTC\n";
    }
    std::cout << "  ------------------------------\n";
    for (const mme::Order& level : bids) {
        std::cout << "  BID  $" << std::setprecision(2) << tick.toReal(level.price)
                  << "  " << std::setprecision(8) << lot.toReal(level.quantity)
                  << " BTC\n";
    }
}

}  // namespace

int main() {
    // BTC-USD on Coinbase: prices move in $0.01 steps and quantities in
    // 0.00000001 BTC steps.
    const mme::TickSize btcUsdTick{0.01};
    const mme::LotSize btcLot{0.00000001};

    mme::OrderBook book;
    mme::FeatureEngine features(btcUsdTick);
    mme::EventRecorder recorder("data/raw/events.csv");
    mme::MarketDataFeed feed("BTC-USD");

    // Every event, live or replayed, goes through this one function.
    const mme::EventHandler handleEvent = [&](const mme::MarketEvent& event) {
        // TODO(Phase 3): If apply() rejects an event or the book becomes
        // crossed, log it and request a fresh snapshot, since the book may no
        // longer match the exchange.
        book.apply(event);
        recorder.record(event);
        // TODO(Phase 6): Compute features here and write them to
        // data/processed/ for the Phase 7 analysis.
        (void)features.compute(book);
    };
    feed.setEventHandler(handleEvent);

    std::cout << "Market Microstructure Research Engine\n"
              << "Status: Phase 2 (order book)\n\n";

    // TODO(Phase 3): Add a command-line option to choose between live mode
    // (MarketDataFeed) and replay mode (ReplayEngine).
    if (!feed.connect()) {
        std::cout << "Live feed for " << feed.symbol()
                  << " is not implemented yet (Phase 3).\n"
                  << "Running a demo event sequence instead.\n\n";
    }

    auto px = [&](double dollars) { return btcUsdTick.toUnits(dollars); };
    auto qty = [&](double btc) { return btcLot.toUnits(btc); };
    using mme::EventType;
    using mme::Side;

    const std::vector<mme::MarketEvent> demo = {
        // Snapshot of the exchange's book when we connect.
        {1, EventType::Snapshot, px(60000.00), qty(2.0), Side::Buy},
        {1, EventType::Snapshot, px(59995.00), qty(4.0), Side::Buy},
        {1, EventType::Snapshot, px(60010.00), qty(1.0), Side::Sell},
        {1, EventType::Snapshot, px(60015.00), qty(3.0), Side::Sell},
        // More sellers join at $60,010.
        {2, EventType::Update, px(60010.00), qty(3.0), Side::Sell},
        // A buyer takes 1 BTC at the best ask; the book itself is unchanged...
        {3, EventType::Trade, px(60010.00), qty(1.0), Side::Buy},
        // ...until the exchange sends the ask's new total.
        {4, EventType::Update, px(60010.00), qty(2.0), Side::Sell},
        // A new, higher bid becomes the best bid.
        {5, EventType::Update, px(60005.00), qty(0.5), Side::Buy},
    };
    for (const mme::MarketEvent& event : demo) {
        handleEvent(event);
    }

    std::cout << "Order book after " << demo.size() << " events:\n";
    printBook(book, btcUsdTick, btcLot, 5);

    const mme::MarketFeatures top = features.compute(book);
    std::cout << std::setprecision(2) << "\nBest bid: $" << *top.bestBid
              << "   Best ask: $" << *top.bestAsk << '\n';
    return 0;
}
