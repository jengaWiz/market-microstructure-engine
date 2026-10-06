#pragma once

#include "market/Units.hpp"

namespace mme {

// Which side of the book an order or price level belongs to.
//   Buy  = a bid (someone willing to buy)
//   Sell = an ask (someone willing to sell)
enum class Side {
    Buy,
    Sell
};

// Human-readable name, handy for logging and for the recorder's file format.
inline const char* toString(Side side) {
    return side == Side::Buy ? "BUY" : "SELL";
}

// A resting quantity at a single price on one side of the book.
//
// In a Level 2 (L2) book we never see individual orders from individual
// traders. We only see the *total* quantity at each price. So in practice an
// Order here usually describes one aggregated price level, for example
// "5.2 BTC bid at 60000.00".
//
// Price is in ticks and quantity is in lots (see Units.hpp).
struct Order {
    Price price = 0;
    Quantity quantity = 0;
    Side side = Side::Buy;
};

}  // namespace mme
