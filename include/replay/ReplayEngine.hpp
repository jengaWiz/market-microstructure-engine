#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "market/MarketEvent.hpp"

namespace mme {

// Reads recorded MarketEvents from disk and replays them in timestamp order.
//
// Replayed events go to the same kind of EventHandler that the live
// MarketDataFeed uses, so the OrderBook and FeatureEngine cannot tell whether
// data is live or replayed. That is what makes offline research repeatable.
//
// Replay runs as fast as possible (no sleeping between events). Replay is
// deterministic: the same file always produces the same sequence of events.
//
// Current status: placeholder. Loading from disk is not implemented.
class ReplayEngine {
public:
    // `inputPath` is a file produced by EventRecorder.
    explicit ReplayEngine(std::string inputPath);

    // Sets the function that receives every replayed event.
    void setEventHandler(EventHandler handler);

    // Reads all events from the input file into memory.
    // Returns true on success.
    // TODO(Phase 5): Not implemented; always returns false.
    bool load();

    // Sends every loaded event to the handler, in order.
    // Returns the number of events replayed.
    std::size_t run();

    std::size_t eventCount() const;
    const std::string& inputPath() const;

private:
    std::string inputPath_;
    EventHandler handler_;
    std::vector<MarketEvent> events_;
};

}  // namespace mme
