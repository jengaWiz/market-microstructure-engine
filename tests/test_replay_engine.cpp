#include <gtest/gtest.h>

#include "replay/ReplayEngine.hpp"

namespace mme {

TEST(ReplayEngineTest, NewEngineHasNoEvents) {
    const ReplayEngine replay("data/raw/does-not-exist.csv");

    EXPECT_EQ(replay.eventCount(), 0u);
    EXPECT_EQ(replay.inputPath(), "data/raw/does-not-exist.csv");
}

TEST(ReplayEngineTest, RunWithNoEventsDeliversNothing) {
    ReplayEngine replay("data/raw/does-not-exist.csv");
    int eventsReceived = 0;
    replay.setEventHandler([&](const MarketEvent&) { ++eventsReceived; });

    EXPECT_EQ(replay.run(), 0u);
    EXPECT_EQ(eventsReceived, 0);
}

// TODO(Phase 5): Add tests that record a small file, load it, and check that
// events are replayed in timestamp order.

}  // namespace mme
