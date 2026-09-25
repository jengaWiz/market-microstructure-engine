#include <gtest/gtest.h>

#include <string>

#include "market/MarketEvent.hpp"
#include "market/Order.hpp"

namespace mme {

TEST(OrderTest, StoresPriceQuantityAndSide) {
    const Order order{60000.5, 1.25, Side::Sell};

    EXPECT_DOUBLE_EQ(order.price, 60000.5);
    EXPECT_DOUBLE_EQ(order.quantity, 1.25);
    EXPECT_EQ(order.side, Side::Sell);
}

TEST(OrderTest, SideHasReadableName) {
    EXPECT_EQ(std::string(toString(Side::Buy)), "BUY");
    EXPECT_EQ(std::string(toString(Side::Sell)), "SELL");
}

TEST(MarketEventTest, StoresAllFields) {
    const MarketEvent event{1'700'000'000'000'000'000, EventType::Trade,
                            60000.0, 0.5, Side::Buy};

    EXPECT_EQ(event.timestamp, 1'700'000'000'000'000'000);
    EXPECT_EQ(event.type, EventType::Trade);
    EXPECT_DOUBLE_EQ(event.price, 60000.0);
    EXPECT_DOUBLE_EQ(event.quantity, 0.5);
    EXPECT_EQ(event.side, Side::Buy);
}

TEST(MarketEventTest, EventTypeHasReadableName) {
    EXPECT_EQ(std::string(toString(EventType::Snapshot)), "SNAPSHOT");
    EXPECT_EQ(std::string(toString(EventType::Update)), "UPDATE");
    EXPECT_EQ(std::string(toString(EventType::Trade)), "TRADE");
}

}  // namespace mme
