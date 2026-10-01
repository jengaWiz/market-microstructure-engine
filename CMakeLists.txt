cmake_minimum_required(VERSION 3.20)

project(MarketMicrostructureEngine
    VERSION 0.1.0
    DESCRIPTION "A beginner-friendly market microstructure research engine"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Lets editors / clangd understand the build.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

option(MME_BUILD_TESTS "Build the unit tests" ON)

# Warning flags shared by every target we own.
if(MSVC)
    set(MME_WARNINGS /W4)
else()
    set(MME_WARNINGS -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Core library: everything except main(). Both the app and the tests link it.
# ---------------------------------------------------------------------------
add_library(mme_core
    src/market/OrderBook.cpp
    src/market/Units.cpp
    src/feed/MarketDataFeed.cpp
    src/recorder/EventRecorder.cpp
    src/replay/ReplayEngine.cpp
    src/research/FeatureEngine.cpp
)

target_include_directories(mme_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(mme_core PRIVATE ${MME_WARNINGS})

# ---------------------------------------------------------------------------
# Application executable
# ---------------------------------------------------------------------------
add_executable(mme src/main.cpp)
target_link_libraries(mme PRIVATE mme_core)
target_compile_options(mme PRIVATE ${MME_WARNINGS})

# ---------------------------------------------------------------------------
# Tests (GoogleTest, downloaded at configure time)
# ---------------------------------------------------------------------------
if(MME_BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # On Windows, keep GoogleTest from overriding our runtime library settings.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # We only need GoogleTest itself: skip GoogleMock and install rules.
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(mme_tests
        tests/test_market_event.cpp
        tests/test_units.cpp
        tests/test_order_book.cpp
        tests/test_replay_engine.cpp
        tests/test_feature_engine.cpp
    )
    target_link_libraries(mme_tests PRIVATE mme_core GTest::gtest_main)
    target_compile_options(mme_tests PRIVATE ${MME_WARNINGS})

    include(GoogleTest)
    gtest_discover_tests(mme_tests)
endif()
