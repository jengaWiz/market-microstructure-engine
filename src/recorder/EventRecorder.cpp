#include "recorder/EventRecorder.hpp"

#include <utility>

namespace mme {

EventRecorder::EventRecorder(std::string outputPath)
    : outputPath_(std::move(outputPath)) {}

bool EventRecorder::open() {
    // TODO(Phase 4): Open outputPath_ with std::ofstream and write a header
    // line. A simple CSV is a good first format:
    //   timestamp,type,side,price,quantity
    open_ = false;
    return open_;
}

void EventRecorder::record(const MarketEvent& event) {
    // TODO(Phase 4): Write one line per event and increment eventsRecorded_.
    // Use toString(event.type) and toString(event.side) for readable columns,
    // and enough decimal places that prices round-trip exactly.
    (void)event;
}

void EventRecorder::close() {
    // TODO(Phase 4): Flush and close the file stream.
    open_ = false;
}

bool EventRecorder::isOpen() const {
    return open_;
}

std::size_t EventRecorder::eventsRecorded() const {
    return eventsRecorded_;
}

const std::string& EventRecorder::outputPath() const {
    return outputPath_;
}

}  // namespace mme
