#pragma once

#include <cstddef>
#include <string>

#include "market/MarketEvent.hpp"

namespace mme {

// Writes MarketEvents to disk so they can be replayed later.
//
// Recording the *normalized* MarketEvents (not raw exchange JSON) means the
// ReplayEngine only ever has to read one simple format.
//
// Current status: placeholder. Nothing is written to disk yet.
class EventRecorder {
public:
    // `outputPath` is the file to write, e.g. "data/raw/btc-usd.csv".
    explicit EventRecorder(std::string outputPath);

    // Opens the output file. Returns true on success.
    // TODO(Phase 4): Not implemented; always returns false.
    bool open();

    // Appends one event to the output file.
    // TODO(Phase 4): Not implemented; the event is ignored.
    void record(const MarketEvent& event);

    // Flushes and closes the output file. Safe to call when not open.
    void close();

    bool isOpen() const;
    std::size_t eventsRecorded() const;
    const std::string& outputPath() const;

private:
    std::string outputPath_;
    bool open_ = false;
    std::size_t eventsRecorded_ = 0;
};

}  // namespace mme
