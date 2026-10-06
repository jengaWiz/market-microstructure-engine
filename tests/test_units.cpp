#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>

#include "market/Units.hpp"

namespace mme {

TEST(IncrementTest, ConvertsPriceToTicks) {
    const TickSize tick{0.01};

    EXPECT_EQ(tick.toUnits(60000.00), 6'000'000);
    EXPECT_EQ(tick.toUnits(60000.01), 6'000'001);
    EXPECT_EQ(tick.toUnits(0.01), 1);
}

TEST(IncrementTest, RoundsInsteadOfTruncating) {
    const TickSize tick{0.01};

    // 60000.05 / 0.01 is 6000004.9999... in floating point. Truncating would
    // give 6'000'004, one cent too low.
    EXPECT_EQ(tick.toUnits(60000.05), 6'000'005);

    // 0.1 + 0.2 is 0.30000000000000004 as a double, but still 30 cents.
    EXPECT_EQ(tick.toUnits(0.1 + 0.2), 30);
}

TEST(IncrementTest, EqualPricesGiveEqualTicks) {
    const TickSize tick{0.01};

    // As doubles these two are NOT equal; as ticks they are.
    EXPECT_NE(0.1 + 0.2, 0.3);
    EXPECT_EQ(tick.toUnits(0.1 + 0.2), tick.toUnits(0.3));
}

TEST(IncrementTest, ConvertsQuantityToLots) {
    const LotSize lot{0.00000001};  // one satoshi

    EXPECT_EQ(lot.toUnits(2.5), 250'000'000);
    EXPECT_EQ(lot.toUnits(0.00000001), 1);
    EXPECT_EQ(lot.toUnits(0.0), 0);
}

TEST(IncrementTest, ConvertsBackToRealValues) {
    const TickSize tick{0.01};
    const LotSize lot{0.00000001};

    EXPECT_DOUBLE_EQ(tick.toReal(6'000'005), 60000.05);
    EXPECT_DOUBLE_EQ(lot.toReal(250'000'000), 2.5);
}

TEST(IncrementTest, RoundTripsExactly) {
    const TickSize tick{0.01};

    // Ticks -> real -> ticks must give back the same whole number, which is
    // what makes recorded data replay exactly.
    for (Price ticks : {Price{1}, Price{6'000'005}, Price{9'999'999'999}}) {
        EXPECT_EQ(tick.toUnits(tick.toReal(ticks)), ticks);
    }
}

TEST(IncrementTest, RejectsInvalidSizes) {
    EXPECT_THROW(Increment{0.0}, std::invalid_argument);
    EXPECT_THROW(Increment{-0.01}, std::invalid_argument);
    EXPECT_THROW(Increment{std::nan("")}, std::invalid_argument);
    EXPECT_THROW(Increment{std::numeric_limits<double>::infinity()},
                 std::invalid_argument);
}

TEST(IncrementTest, RejectsValuesThatDoNotFit) {
    const TickSize tick{0.01};

    EXPECT_THROW((void)tick.toUnits(1e30), std::out_of_range);
    EXPECT_THROW((void)tick.toUnits(-1e30), std::out_of_range);
    EXPECT_THROW((void)tick.toUnits(std::nan("")), std::out_of_range);
    EXPECT_THROW((void)tick.toUnits(std::numeric_limits<double>::infinity()),
                 std::out_of_range);
}

TEST(IncrementTest, ReportsItsSize) {
    EXPECT_DOUBLE_EQ(TickSize{0.01}.size(), 0.01);
}

}  // namespace mme
