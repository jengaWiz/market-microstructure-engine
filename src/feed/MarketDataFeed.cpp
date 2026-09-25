#include "feed/MarketDataFeed.hpp"

#include <utility>

namespace mme {

MarketDataFeed::MarketDataFeed(std::string symbol)
    : symbol_(std::move(symbol)) {}

void MarketDataFeed::setEventHandler(EventHandler handler) {
    handler_ = std::move(handler);
}

bool MarketDataFeed::connect() {
    // TODO(Phase 3): Connect to the exchange WebSocket.
    //   1. Choose a small WebSocket + JSON library (decide in Phase 3).
    //   2. Open the connection and subscribe to the L2 channel for symbol_.
    //   3. For each incoming message, translate it into one or more
    //      MarketEvents (a snapshot message becomes many Snapshot events)
    //      and call publish() for each.
    //   4. Handle disconnects and sequence gaps by requesting a new snapshot.
    connected_ = false;
    return connected_;
}

void MarketDataFeed::disconnect() {
    // TODO(Phase 3): Close the WebSocket if it is open.
    connected_ = false;
}

bool MarketDataFeed::isConnected() const {
    return connected_;
}

const std::string& MarketDataFeed::symbol() const {
    return symbol_;
}

void MarketDataFeed::publish(const MarketEvent& event) const {
    if (handler_) {
        handler_(event);
    }
}

}  // namespace mme
