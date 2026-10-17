// Session 2: bounded capacity and interpolation.
// Run: ctest --preset dev -L ex01-s2

#include "sensorSync/odometryBuffer.h"
#include "testHelpers.h"

#include <gtest/gtest.h>

namespace sensorSync::test
{
namespace
{

using namespace std::chrono_literals;

constexpr std::size_t kLargeCapacity = 1000;
constexpr double kPositionToleranceMeters = 1e-9;
constexpr double kYawToleranceRad = 1e-9;

// ---- Capacity --------------------------------------------------------------------------------------------------

TEST(OdometryBufferCapacity, EvictsOldestWhenFull)
{
    OdometryBuffer buffer{2};
    buffer.add(makeOdometry(at(0ms)));
    buffer.add(makeOdometry(at(20ms)));
    EXPECT_TRUE(buffer.add(makeOdometry(at(40ms))));

    EXPECT_EQ(buffer.size(), 2U);
    EXPECT_EQ(buffer.droppedCount(), 1U);
    auto const oldest = buffer.findNearest(at(0ms), 1s);
    ASSERT_TRUE(oldest.has_value());
    EXPECT_EQ(oldest->timestampNs, at(20ms));
}

TEST(OdometryBufferCapacity, RejectedSampleDoesNotEvict)
{
    OdometryBuffer buffer{2};
    buffer.add(makeOdometry(at(0ms)));
    buffer.add(makeOdometry(at(20ms)));
    EXPECT_FALSE(buffer.add(makeOdometry(at(10ms))));

    EXPECT_EQ(buffer.size(), 2U);
    EXPECT_EQ(buffer.droppedCount(), 0U);
    EXPECT_EQ(buffer.rejectedCount(), 1U);
}

TEST(OdometryBufferCapacity, StaysBoundedUnderLongStreams)
{
    constexpr std::size_t kCapacity = 50;
    constexpr int kSampleCount = 10'000;
    OdometryBuffer buffer{kCapacity};
    for (int i = 0; i < kSampleCount; ++i)
    {
        buffer.add(makeOdometry(at(std::chrono::milliseconds{20 * i})));
    }
    EXPECT_EQ(buffer.size(), kCapacity);
    EXPECT_EQ(buffer.droppedCount(), kSampleCount - kCapacity);
}

// ---- Interpolation ---------------------------------------------------------------------------------------------

TEST(OdometryBufferInterpolate, EmptyBufferHasNoPose)
{
    OdometryBuffer const buffer{kLargeCapacity};
    EXPECT_FALSE(buffer.interpolate(kT0, 1s).has_value());
}

TEST(OdometryBufferInterpolate, MidpointPosition)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 0.0, 10.0));
    buffer.add(makeOdometry(at(20ms), 2.0, 6.0));

    auto const pose = buffer.interpolate(at(10ms), 50ms);
    ASSERT_TRUE(pose.has_value());
    EXPECT_EQ(pose->timestampNs, at(10ms));
    EXPECT_NEAR(pose->xMeters, 1.0, kPositionToleranceMeters);
    EXPECT_NEAR(pose->yMeters, 8.0, kPositionToleranceMeters);
}

TEST(OdometryBufferInterpolate, UsesTheBracketingPair)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 0.0));
    buffer.add(makeOdometry(at(20ms), 1.0));
    buffer.add(makeOdometry(at(40ms), 5.0));

    auto const pose = buffer.interpolate(at(25ms), 50ms);
    ASSERT_TRUE(pose.has_value());
    EXPECT_NEAR(pose->xMeters, 2.0, kPositionToleranceMeters);
}

TEST(OdometryBufferInterpolate, ExactSampleIsReturnedUnchanged)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 1.0, 2.0, 0.5));
    buffer.add(makeOdometry(at(20ms), 3.0, 4.0, 0.7));

    auto const first = buffer.interpolate(at(0ms), 50ms);
    ASSERT_TRUE(first.has_value());
    EXPECT_DOUBLE_EQ(first->xMeters, 1.0);
    EXPECT_DOUBLE_EQ(first->yawRad, 0.5);

    auto const last = buffer.interpolate(at(20ms), 50ms);
    ASSERT_TRUE(last.has_value());
    EXPECT_DOUBLE_EQ(last->yMeters, 4.0);
    EXPECT_DOUBLE_EQ(last->yawRad, 0.7);
}

TEST(OdometryBufferInterpolate, OutsideStoredRangeHasNoPose)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(10ms)));
    buffer.add(makeOdometry(at(30ms)));

    EXPECT_FALSE(buffer.interpolate(at(9ms), 1s).has_value());
    EXPECT_FALSE(buffer.interpolate(at(31ms), 1s).has_value());
}

TEST(OdometryBufferInterpolate, GapLargerThanMaxGapHasNoPose)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms)));
    buffer.add(makeOdometry(at(100ms)));

    EXPECT_FALSE(buffer.interpolate(at(50ms), 99ms).has_value());
    EXPECT_TRUE(buffer.interpolate(at(50ms), 100ms).has_value());
}

TEST(OdometryBufferInterpolate, YawWithoutWrapAround)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 0.0, 0.0, 0.1));
    buffer.add(makeOdometry(at(20ms), 0.0, 0.0, 0.3));

    auto const pose = buffer.interpolate(at(10ms), 50ms);
    ASSERT_TRUE(pose.has_value());
    EXPECT_NEAR(pose->yawRad, 0.2, kYawToleranceRad);
}

TEST(OdometryBufferInterpolate, YawTakesShortestArcAcrossPi)
{
    // Turning left through the +pi/-pi seam: 3.0 rad -> -3.0 rad is a 0.28 rad turn, not a 6 rad one.
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 0.0, 0.0, 3.0));
    buffer.add(makeOdometry(at(20ms), 0.0, 0.0, -3.0));

    auto const pose = buffer.interpolate(at(10ms), 50ms);
    ASSERT_TRUE(pose.has_value());
    EXPECT_NEAR(angleDifference(pose->yawRad, kPi), 0.0, kYawToleranceRad);
}

TEST(OdometryBufferInterpolate, YawResultIsNormalised)
{
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 0.0, 0.0, 3.1));
    buffer.add(makeOdometry(at(20ms), 0.0, 0.0, -3.1));

    auto const pose = buffer.interpolate(at(15ms), 50ms);
    ASSERT_TRUE(pose.has_value());
    EXPECT_GT(pose->yawRad, -kPi);
    EXPECT_LE(pose->yawRad, kPi);
    double const expectedRad = 3.1 + 0.75 * (2.0 * kPi - 6.2) - 2.0 * kPi;
    EXPECT_NEAR(pose->yawRad, expectedRad, kYawToleranceRad);
}

TEST(OdometryBufferInterpolate, PreciseAtEpochTimestamps)
{
    // A quarter of the way between two samples. Converting absolute epoch nanoseconds to double before
    // subtracting loses tens of nanoseconds, which shows up as a visible position error here.
    OdometryBuffer buffer{kLargeCapacity};
    buffer.add(makeOdometry(at(0ms), 0.0));
    buffer.add(makeOdometry(at(20ms), 1.0));

    auto const pose = buffer.interpolate(at(5ms), 50ms);
    ASSERT_TRUE(pose.has_value());
    EXPECT_NEAR(pose->xMeters, 0.25, kPositionToleranceMeters);
}

} // namespace
} // namespace sensorSync::test
