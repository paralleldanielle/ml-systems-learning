// Session 1: storing samples and finding the nearest one.
// Run: ctest --preset dev -L ex01-s1

#include "sensorSync/odometryBuffer.h"
#include "testHelpers.h"

#include <gtest/gtest.h>

namespace sensorSync::test
{
namespace
{

using namespace std::chrono_literals;

// Large enough that session-1 tests never hit the capacity limit.
constexpr std::size_t kLargeCapacity = 1000;

TEST(OdometryBufferSession1, EmptyBufferHasNoMatch)
{
    OdometryBuffer const buffer{kLargeCapacity};
    EXPECT_EQ(buffer.size(), 0U);
    EXPECT_FALSE(buffer.findNearest(kT0, 1s).has_value());
}

TEST(OdometryBufferSession1, AddStoresInOrderSamples)
{
    OdometryBuffer buffer{kLargeCapacity};
    EXPECT_TRUE(buffer.add(makeOdometry(at(0ms))));
    EXPECT_TRUE(buffer.add(makeOdometry(at(20ms))));
    EXPECT_TRUE(buffer.add(makeOdometry(at(40ms))));
    EXPECT_EQ(buffer.size(), 3U);
    EXPECT_EQ(buffer.rejectedCount(), 0U);
}

TEST(OdometryBufferSession1, RejectsOutOfOrderSample)
{
    OdometryBuffer buffer{kLargeCapacity};
    ASSERT_TRUE(buffer.add(makeOdometry(at(40ms))));
    EXPECT_FALSE(buffer.add(makeOdometry(at(20ms))));
    EXPECT_EQ(buffer.size(), 1U);
    EXPECT_EQ(buffer.rejectedCount(), 1U);
}

TEST(OdometryBufferSession1, RejectsDuplicateTimestamp)
{
    OdometryBuffer buffer{kLargeCapacity};
    ASSERT_TRUE(buffer.add(makeOdometry(at(20ms), 1.0)));
    EXPECT_FALSE(buffer.add(makeOdometry(at(20ms), 2.0)));
    EXPECT_EQ(buffer.size(), 1U);
    EXPECT_EQ(buffer.rejectedCount(), 1U);
}

TEST(OdometryBufferSession1, ExactTimestampMatches)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 0.0));
    buffer.add(makeOdometry(at(20ms), 1.0));
    buffer.add(makeOdometry(at(40ms), 2.0));

    auto const match = buffer.findNearest(at(20ms), 0ns);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->timestampNs, at(20ms));
    EXPECT_DOUBLE_EQ(match->xMeters, 1.0);
}

TEST(OdometryBufferSession1, ReturnsTheWholeSample)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 1.5, -2.5, 0.25));

    auto const match = buffer.findNearest(at(1ms), 5ms);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->timestampNs, at(0ms));
    EXPECT_DOUBLE_EQ(match->xMeters, 1.5);
    EXPECT_DOUBLE_EQ(match->yMeters, -2.5);
    EXPECT_DOUBLE_EQ(match->yawRad, 0.25);
}

TEST(OdometryBufferSession1, PicksCloserOfTwoSamples)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms)));
    buffer.add(makeOdometry(at(20ms)));

    auto const closerToEarlier = buffer.findNearest(at(7ms), 20ms);
    ASSERT_TRUE(closerToEarlier.has_value());
    EXPECT_EQ(closerToEarlier->timestampNs, at(0ms));

    auto const closerToLater = buffer.findNearest(at(13ms), 20ms);
    ASSERT_TRUE(closerToLater.has_value());
    EXPECT_EQ(closerToLater->timestampNs, at(20ms));
}

TEST(OdometryBufferSession1, TieGoesToEarlierSample)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms)));
    buffer.add(makeOdometry(at(20ms)));

    auto const match = buffer.findNearest(at(10ms), 20ms);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->timestampNs, at(0ms));
}

TEST(OdometryBufferSession1, ToleranceIsInclusive)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms)));

    EXPECT_TRUE(buffer.findNearest(at(5ms), 5ms).has_value());
}

TEST(OdometryBufferSession1, JustOutsideToleranceHasNoMatch)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms)));

    EXPECT_FALSE(buffer.findNearest(at(5ms) + 1, 5ms).has_value());
}

TEST(OdometryBufferSession1, QueryBeforeOldestSample)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(10ms)));
    buffer.add(makeOdometry(at(30ms)));

    auto const match = buffer.findNearest(at(8ms), 5ms);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->timestampNs, at(10ms));
}

TEST(OdometryBufferSession1, QueryAfterNewestSample)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(10ms)));
    buffer.add(makeOdometry(at(30ms)));

    auto const match = buffer.findNearest(at(32ms), 5ms);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->timestampNs, at(30ms));

    EXPECT_FALSE(buffer.findNearest(at(50ms), 5ms).has_value());
}

TEST(OdometryBufferSession1, NanosecondResolutionAtEpochTimestamps)
{
    // 1 microsecond apart; the query is 1 ns closer to the second sample. Doubles near 1.7e18 are 256 ns apart,
    // so comparing timestamps as double sees a tie here and picks the wrong sample.
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ns)));
    buffer.add(makeOdometry(at(1000ns)));

    auto const match = buffer.findNearest(at(501ns), 1ms);
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->timestampNs, at(1000ns));
}

TEST(OdometryBufferSession1, DiscardBeforeKeepsSampleAtBoundary)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms)));
    buffer.add(makeOdometry(at(20ms)));
    buffer.add(makeOdometry(at(40ms)));

    buffer.discardBefore(at(20ms));
    EXPECT_EQ(buffer.size(), 2U);

    auto const oldest = buffer.findNearest(at(0ms), 1s);
    ASSERT_TRUE(oldest.has_value());
    EXPECT_EQ(oldest->timestampNs, at(20ms));
}

TEST(OdometryBufferSession1, DiscardBeforeEverything)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms)));
    buffer.add(makeOdometry(at(20ms)));

    buffer.discardBefore(at(1s));
    EXPECT_EQ(buffer.size(), 0U);
    EXPECT_FALSE(buffer.findNearest(at(20ms), 1s).has_value());
}

TEST(OdometryBufferSession1, DiscardBeforeOnEmptyBufferIsSafe)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.discardBefore(kT0);
    EXPECT_EQ(buffer.size(), 0U);
}

} // namespace
} // namespace sensorSync::test
