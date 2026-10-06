#include "market/Units.hpp"

#include <cmath>
#include <stdexcept>

namespace mme {

Increment::Increment(double size) : size_(size) {
    // `!(size > 0.0)` also catches NaN, which `size <= 0.0` would let through.
    if (!(size > 0.0) || !std::isfinite(size)) {
        throw std::invalid_argument("Increment size must be a positive, finite number");
    }
}

std::int64_t Increment::toUnits(double value) const {
    const double steps = std::round(value / size_);

    // 2^63 is the first value an int64 can't hold. Checking before converting
    // matters: converting an out-of-range double to an integer is undefined.
    // `!(... < ...)` also catches NaN.
    constexpr double kLimit = 9223372036854775808.0;  // 2^63
    if (!(steps < kLimit) || !(steps >= -kLimit)) {
        throw std::out_of_range("Value does not fit in a whole number of steps");
    }
    return static_cast<std::int64_t>(steps);
}

double Increment::toReal(std::int64_t units) const {
    return static_cast<double>(units) * size_;
}

double Increment::size() const {
    return size_;
}

}  // namespace mme
