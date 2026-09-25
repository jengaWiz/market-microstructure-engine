#include "replay/ReplayEngine.hpp"

#include <utility>

namespace mme {

ReplayEngine::ReplayEngine(std::string inputPath)
    : inputPath_(std::move(inputPath)) {}

void ReplayEngine::setEventHandler(EventHandler handler) {
    handler_ = std::move(handler);
}

bool ReplayEngine::load() {
    // TODO(Phase 5): Read inputPath_ (the format written by EventRecorder),
    // parse each line into a MarketEvent, and store it in events_.
    // Then sort with std::stable_sort by timestamp so events with equal
    // timestamps keep their recorded order.
    events_.clear();
    return false;
}

std::size_t ReplayEngine::run() {
    if (!handler_) {
        return 0;
    }
    for (const MarketEvent& event : events_) {
        handler_(event);
    }
    return events_.size();
}

std::size_t ReplayEngine::eventCount() const {
    return events_.size();
}

const std::string& ReplayEngine::inputPath() const {
    return inputPath_;
}

}  // namespace mme
