// Measures how many events per second OrderBook::apply() can handle.
//
// Usage: ./build/mme_bench [event_count]     (default: 1,000,000)
//
// Build in Release mode for meaningful numbers:
//   cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
//   cmake --build build-release --target mme_bench
//   ./build-release/mme_bench
//
// The events are generated up front (not timed) to look like a real L2 feed:
// most updates land within a few ticks of the top of the book, some remove a
// level, and bids stay below asks so the book never crosses. A fixed random
// seed makes every run use exactly the same events.

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "market/MarketEvent.hpp"
#include "market/OrderBook.hpp"
#include "market/Units.hpp"

namespace {

using mme::EventType;
using mme::MarketEvent;
using mme::OrderBook;
using mme::Price;
using mme::Quantity;
using mme::Side;

constexpr Price kMidPrice = 6'000'000;  // $60,000.00 in $0.01 ticks
constexpr int kSnapshotLevels = 200;    // levels per side in the starting book
constexpr int kRuns = 5;

// Generates the same "random" stream on every platform. The engine is fully
// specified by the C++ standard; we avoid std::uniform_int_distribution
// because its output differs between standard libraries.
class Random {
public:
    explicit Random(std::uint64_t seed) : engine_(seed) {}

    // A number in [0, n).
    std::uint64_t below(std::uint64_t n) { return engine_() % n; }

private:
    std::mt19937_64 engine_;
};

// A snapshot of `kSnapshotLevels` levels on each side around the mid price.
std::vector<MarketEvent> makeSnapshot() {
    std::vector<MarketEvent> events;
    for (int i = 1; i <= kSnapshotLevels; ++i) {
        events.push_back({0, EventType::Snapshot, kMidPrice - i, 100'000'000, Side::Buy});
        events.push_back({0, EventType::Snapshot, kMidPrice + i, 100'000'000, Side::Sell});
    }
    return events;
}

// `count` update events shaped like a busy L2 feed.
std::vector<MarketEvent> makeUpdates(std::size_t count) {
    Random random(42);
    std::vector<MarketEvent> events;
    events.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const Side side = random.below(2) == 0 ? Side::Buy : Side::Sell;

        // Distance from the mid price, in ticks: 80% within 10 ticks of the
        // top of the book, the rest spread out up to 250 ticks away.
        const Price distance = random.below(10) < 8
                                   ? 1 + static_cast<Price>(random.below(10))
                                   : 1 + static_cast<Price>(random.below(250));
        const Price price = side == Side::Buy ? kMidPrice - distance
                                              : kMidPrice + distance;

        // 20% of updates remove the level; the rest set 0.001 to 5 BTC.
        const Quantity quantity =
            random.below(5) == 0
                ? 0
                : 100'000 + static_cast<Quantity>(random.below(500'000'000));

        events.push_back({static_cast<std::int64_t>(i), EventType::Update,
                          price, quantity, side});
    }
    return events;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::size_t count = 1'000'000;
    if (argc > 1) {
        count = std::strtoull(argv[1], nullptr, 10);
        if (count == 0) {
            std::cerr << "Usage: mme_bench [event_count]\n";
            return 1;
        }
    }

#ifndef NDEBUG
    std::cout << "WARNING: this is a Debug build. Numbers will be much slower\n"
                 "than real performance. Build with -DCMAKE_BUILD_TYPE=Release.\n\n";
#endif

    const std::vector<MarketEvent> snapshot = makeSnapshot();
    const std::vector<MarketEvent> updates = makeUpdates(count);

    // Run several times and report the median, which is less affected by
    // whatever else the computer happens to be doing.
    std::vector<double> nanosPerEvent;
    std::size_t bidLevels = 0;
    std::size_t askLevels = 0;
    for (int run = 0; run < kRuns; ++run) {
        OrderBook book;
        for (const MarketEvent& event : snapshot) {
            book.apply(event);
        }

        const auto start = std::chrono::steady_clock::now();
        for (const MarketEvent& event : updates) {
            book.apply(event);
        }
        const auto end = std::chrono::steady_clock::now();

        const double nanos =
            std::chrono::duration<double, std::nano>(end - start).count();
        nanosPerEvent.push_back(nanos / static_cast<double>(count));

        // Printing the final book size also stops the compiler from treating
        // the loop as unused work and skipping it.
        bidLevels = book.bidLevelCount();
        askLevels = book.askLevelCount();
    }

    std::sort(nanosPerEvent.begin(), nanosPerEvent.end());
    const double median = nanosPerEvent[kRuns / 2];

    std::cout << "OrderBook::apply() benchmark\n"
              << "  events per run:  " << count << " updates\n"
              << "  runs:            " << kRuns << " (median reported)\n"
              << "  final book:      " << bidLevels << " bids, " << askLevels
              << " asks\n"
              << "  ns per event:    " << std::fixed << std::setprecision(1)
              << median << '\n'
              << "  events / second: "
              << static_cast<std::uint64_t>(1e9 / median) << '\n';
    return 0;
}
