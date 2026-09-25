// Entry point. For now this only wires the components together to show the
// intended pipeline:
//
//   MarketDataFeed -> MarketEvent -> OrderBook -> EventRecorder
//                                             \-> FeatureEngine
//
// No network connection is made and nothing is written to disk yet.

#include <iostream>

#include "feed/MarketDataFeed.hpp"
#include "market/MarketEvent.hpp"
#include "market/OrderBook.hpp"
#include "recorder/EventRecorder.hpp"
#include "research/FeatureEngine.hpp"

int main() {
    mme::OrderBook book;
    mme::FeatureEngine features;
    mme::EventRecorder recorder("data/raw/events.csv");
    mme::MarketDataFeed feed("BTC-USD");

    // Every event, live or replayed, goes through this one function.
    feed.setEventHandler([&](const mme::MarketEvent& event) {
        book.apply(event);
        recorder.record(event);
        // TODO(Phase 6): Compute features here and write them to
        // data/processed/ for the Phase 7 analysis.
        (void)features.compute(book);
    });

    std::cout << "Market Microstructure Research Engine\n"
              << "Status: Phase 1 (project skeleton)\n\n";

    // TODO(Phase 3): Add a command-line option to choose between live mode
    // (MarketDataFeed) and replay mode (ReplayEngine).
    if (!feed.connect()) {
        std::cout << "Live feed for " << feed.symbol()
                  << " is not implemented yet (Phase 3).\n";
    }

    std::cout << "Order book is " << (book.empty() ? "empty" : "not empty")
              << ".\n";
    return 0;
}
