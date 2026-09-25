#pragma once

#include <string>

#include "market/MarketEvent.hpp"

namespace mme {

// Connects to an exchange and turns its messages into MarketEvents.
//
// This is the ONLY place that should know about a specific exchange's
// WebSocket URL, message format, or quirks. Everything it hands to the rest of
// the system is a plain MarketEvent.
//
// Current status: placeholder. It does not open any network connection.
class MarketDataFeed {
public:
    // `symbol` is the instrument to subscribe to, e.g. "BTC-USD".
    explicit MarketDataFeed(std::string symbol);

    // Sets the function that receives every event this feed produces.
    void setEventHandler(EventHandler handler);

    // Opens the connection and subscribes to market data.
    // Returns true on success.
    // TODO(Phase 3): Not implemented; always returns false.
    bool connect();

    // Closes the connection. Safe to call when not connected.
    void disconnect();

    bool isConnected() const;
    const std::string& symbol() const;

private:
    // Sends an event to the handler, if one is set.
    void publish(const MarketEvent& event) const;

    std::string symbol_;
    EventHandler handler_;
    bool connected_ = false;
};

}  // namespace mme
