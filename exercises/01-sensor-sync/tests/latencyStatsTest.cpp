// Session 2: latency/jitter statistics.
// Run: ctest --preset dev -L ex01-s2

#include "sensorSync/latencyStats.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <vector>

namespace sensorSync::test
{
namespace
{

using namespace std::chrono_literals;

TEST(LatencyStats, EmptyHasNoStatistics)
{
    LatencyStats const stats;
    EXPECT_EQ(stats.count(), 0U);
    EXPECT_FALSE(stats.percentile(50.0).has_value());
    EXPECT_FALSE(stats.max().has_value());
}

TEST(LatencyStats, SingleValueIsEveryPercentile)
{
    LatencyStats stats;
    stats.record(7ms);
    EXPECT_EQ(stats.count(), 1U);
    EXPECT_EQ(stats.percentile(1.0), 7ms);
    EXPECT_EQ(stats.percentile(50.0), 7ms);
    EXPECT_EQ(stats.percentile(100.0), 7ms);
    EXPECT_EQ(stats.max(), 7ms);
}

TEST(LatencyStats, NearestRankOnSmallSample)
{
    LatencyStats stats;
    for (auto const value : {30ms, 10ms, 40ms, 20ms})
    {
        stats.record(value);
    }
    EXPECT_EQ(stats.percentile(25.0), 10ms);
    EXPECT_EQ(stats.percentile(50.0), 20ms);
    EXPECT_EQ(stats.percentile(75.0), 30ms);
    EXPECT_EQ(stats.percentile(76.0), 40ms);
    EXPECT_EQ(stats.percentile(100.0), 40ms);
}

TEST(LatencyStats, NearestRankOnHundredValuesInRandomOrder)
{
    std::vector<std::chrono::nanoseconds> values;
    for (int i = 1; i <= 100; ++i)
    {
        values.emplace_back(std::chrono::milliseconds{i});
    }
    std::mt19937 generator{42};
    std::shuffle(values.begin(), values.end(), generator);

    LatencyStats stats;
    for (auto const value : values)
    {
        stats.record(value);
    }
    EXPECT_EQ(stats.count(), 100U);
    EXPECT_EQ(stats.percentile(1.0), 1ms);
    EXPECT_EQ(stats.percentile(50.0), 50ms);
    EXPECT_EQ(stats.percentile(99.0), 99ms);
    EXPECT_EQ(stats.percentile(100.0), 100ms);
    EXPECT_EQ(stats.max(), 100ms);
}

TEST(LatencyStats, NegativeValuesAreAllowed)
{
    LatencyStats stats;
    stats.record(-5ms);
    stats.record(-1ms);
    EXPECT_EQ(stats.percentile(50.0), -5ms);
    EXPECT_EQ(stats.max(), -1ms);
}

TEST(LatencyStats, QueriesDoNotChangeTheData)
{
    LatencyStats stats;
    for (auto const value : {3ms, 1ms, 2ms})
    {
        stats.record(value);
    }
    EXPECT_EQ(stats.percentile(50.0), 2ms);
    EXPECT_EQ(stats.percentile(50.0), 2ms);
    EXPECT_EQ(stats.count(), 3U);
    stats.record(0ms);
    EXPECT_EQ(stats.percentile(25.0), 0ms);
}

} // namespace
} // namespace sensorSync::test
