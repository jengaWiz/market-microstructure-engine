#include <gtest/gtest.h>

#include <string>

#include "market/MarketEvent.hpp"
#include "market/Order.hpp"

namespace mme {

TEST(OrderTest, StoresPriceQuantityAndSide) {
    // $60,000.50 in $0.01 ticks, 1.25 BTC in 1e-8 BTC lots.
    const Order order{6'000'050, 125'000'000, Side::Sell};

    EXPECT_EQ(order.price, 6'000'050);
    EXPECT_EQ(order.quantity, 125'000'000);
    EXPECT_EQ(order.side, Side::Sell);
}

TEST(OrderTest, SideHasReadableName) {
    EXPECT_EQ(std::string(toString(Side::Buy)), "BUY");
    EXPECT_EQ(std::string(toString(Side::Sell)), "SELL");
}

TEST(MarketEventTest, StoresAllFields) {
    const MarketEvent event{1'700'000'000'000'000'000, EventType::Trade,
                            6'000'000, 50'000'000, Side::Buy};

    EXPECT_EQ(event.timestamp, 1'700'000'000'000'000'000);
    EXPECT_EQ(event.type, EventType::Trade);
    EXPECT_EQ(event.price, 6'000'000);
    EXPECT_EQ(event.quantity, 50'000'000);
    EXPECT_EQ(event.side, Side::Buy);
}

TEST(MarketEventTest, EventTypeHasReadableName) {
    EXPECT_EQ(std::string(toString(EventType::Snapshot)), "SNAPSHOT");
    EXPECT_EQ(std::string(toString(EventType::Update)), "UPDATE");
    EXPECT_EQ(std::string(toString(EventType::Trade)), "TRADE");
}

}  // namespace mme
