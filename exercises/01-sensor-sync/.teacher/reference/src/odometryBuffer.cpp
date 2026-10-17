#include "sensorSync/odometryBuffer.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>

namespace sensorSync
{
namespace
{

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

//! |a - b| for unsigned values without wrapping around when b > a.
std::uint64_t absoluteDifference(std::uint64_t a, std::uint64_t b)
{
    return a > b ? a - b : b - a;
}

//! A non-negative duration as an unsigned nanosecond count; negative durations become std::nullopt.
std::optional<std::uint64_t> toUnsignedNs(std::chrono::nanoseconds duration)
{
    if (duration.count() < 0)
    {
        return std::nullopt;
    }
    return static_cast<std::uint64_t>(duration.count());
}

//! Normalise an angle to (-pi, pi].
double wrapAngle(double angleRad)
{
    double wrapped = std::remainder(angleRad, kTwoPi); // [-pi, pi]
    if (wrapped <= -kPi)
    {
        wrapped += kTwoPi;
    }
    return wrapped;
}

} // namespace

OdometryBuffer::OdometryBuffer(std::size_t capacity) : mCapacity{capacity}
{
    assert(capacity > 0 && "OdometryBuffer capacity must be greater than zero");
}

bool OdometryBuffer::add(Odometry const& sample)
{
    if (!mSamples.empty() && sample.timestampNs <= mSamples.back().timestampNs)
    {
        ++mRejectedCount;
        return false;
    }
    if (mSamples.size() == mCapacity)
    {
        mSamples.pop_front();
        ++mDroppedCount;
    }
    mSamples.push_back(sample);
    return true;
}

std::optional<Odometry> OdometryBuffer::findNearest(std::uint64_t timestampNs, std::chrono::nanoseconds tolerance) const
{
    auto const toleranceNs = toUnsignedNs(tolerance);
    if (!toleranceNs)
    {
        return std::nullopt;
    }

    std::optional<Odometry> best;
    std::uint64_t bestDistanceNs = 0;
    for (Odometry const& sample : mSamples)
    {
        std::uint64_t const distanceNs = absoluteDifference(sample.timestampNs, timestampNs);
        // Strictly smaller keeps the earlier sample on a tie.
        if (distanceNs <= *toleranceNs && (!best || distanceNs < bestDistanceNs))
        {
            best = sample;
            bestDistanceNs = distanceNs;
        }
    }
    return best;
}

void OdometryBuffer::discardBefore(std::uint64_t timestampNs)
{
    while (!mSamples.empty() && mSamples.front().timestampNs < timestampNs)
    {
        mSamples.pop_front();
    }
}

std::size_t OdometryBuffer::size() const
{
    return mSamples.size();
}

std::uint64_t OdometryBuffer::rejectedCount() const
{
    return mRejectedCount;
}

std::optional<Odometry> OdometryBuffer::interpolate(std::uint64_t timestampNs, std::chrono::nanoseconds maxGap) const
{
    auto const maxGapNs = toUnsignedNs(maxGap);
    if (!maxGapNs || mSamples.empty() || timestampNs < mSamples.front().timestampNs
        || timestampNs > mSamples.back().timestampNs)
    {
        return std::nullopt;
    }

    auto const next = std::lower_bound(mSamples.begin(), mSamples.end(), timestampNs,
        [](Odometry const& sample, std::uint64_t t) { return sample.timestampNs < t; });
    if (next->timestampNs == timestampNs)
    {
        return *next;
    }
    Odometry const& before = *std::prev(next);
    Odometry const& after = *next;

    // Subtract in integers first: absolute epoch nanoseconds do not fit exactly in a double.
    std::uint64_t const gapNs = after.timestampNs - before.timestampNs;
    if (gapNs > *maxGapNs)
    {
        return std::nullopt;
    }
    double const fraction = static_cast<double>(timestampNs - before.timestampNs) / static_cast<double>(gapNs);

    Odometry result;
    result.timestampNs = timestampNs;
    result.xMeters = before.xMeters + fraction * (after.xMeters - before.xMeters);
    result.yMeters = before.yMeters + fraction * (after.yMeters - before.yMeters);
    double const yawDeltaRad = wrapAngle(after.yawRad - before.yawRad); // shortest arc
    result.yawRad = wrapAngle(before.yawRad + fraction * yawDeltaRad);
    return result;
}

std::uint64_t OdometryBuffer::droppedCount() const
{
    return mDroppedCount;
}

} // namespace sensorSync
