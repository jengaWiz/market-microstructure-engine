#pragma once

#include <cstdint>

namespace mme {

// Prices and quantities are stored as whole numbers, the way real trading
// systems store them.
//
// Exchanges only allow values on a fixed grid. For BTC-USD on Coinbase:
//   - prices move in steps of $0.01        (the "tick size")
//   - quantities move in steps of 1e-8 BTC (the "lot size", one satoshi)
//
// So every valid value is a whole number of steps. Storing that whole number
// instead of a double means:
//   - equal values always compare equal (with doubles, 0.1 + 0.2 != 0.3)
//   - prices are safe to use as std::map keys
//   - recorded data replays exactly, with no rounding drift
//
// Examples:
//   $60,000.05 with a $0.01 tick -> Price    6'000'005
//   2.5 BTC with a 1e-8 BTC lot  -> Quantity 250'000'000

// A price, measured in ticks.
using Price = std::int64_t;

// A quantity, measured in lots (the smallest tradable unit).
using Quantity = std::int64_t;

// The size of one step on an exchange's price or quantity grid. Converts
// between real values (e.g. 60000.05) and whole numbers of steps
// (e.g. 6'000'005).
//
// The engine works in whole numbers internally. Conversion only happens at the
// edges: when the feed reads exchange messages, and when the FeatureEngine
// reports results in real units for research.
class Increment {
public:
    // `size` is one step, e.g. 0.01 for one cent.
    // Throws std::invalid_argument if `size` is not a positive, finite number.
    explicit Increment(double size);

    // Rounds a real value to the nearest whole number of steps,
    // e.g. 60000.05 -> 6'000'005 with a 0.01 step.
    //
    // Rounding (instead of truncating) matters: 60000.05 / 0.01 is actually
    // 6000004.9999... in floating point, which truncation would get wrong.
    //
    // Throws std::out_of_range if the result doesn't fit in 64 bits
    // (or `value` is NaN or infinite).
    //
    // TODO(Phase 3): Exchanges send numbers as text ("60000.05"). Parsing the
    // text straight into whole steps would avoid floating point completely.
    std::int64_t toUnits(double value) const;

    // Converts a whole number of steps back to a real value,
    // e.g. 6'000'005 -> 60000.05 with a 0.01 step.
    // Use this only for display and research output, never for comparisons.
    double toReal(std::int64_t units) const;

    double size() const;

private:
    double size_;
};

// Names that say which grid an Increment describes.
using TickSize = Increment;  // price step,    e.g. 0.01 USD
using LotSize = Increment;   // quantity step, e.g. 0.00000001 BTC

}  // namespace mme
