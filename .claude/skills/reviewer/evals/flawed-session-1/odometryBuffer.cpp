// Eval fixture for the reviewer skill: a session-1 submission with seeded flaws (see expected.md).
// It compiles cleanly under the dev preset; some session-1 tests fail.
#include "sensorSync/odometryBuffer.h"

#include <cassert>

namespace sensorSync
{

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
    mSamples.push_back(sample);
    return true;
}

std::optional<Odometry> OdometryBuffer::findNearest(std::uint64_t timestampNs, std::chrono::nanoseconds tolerance) const
{
    std::optional<Odometry> best;
    std::uint64_t bestDistance = 18446744073709551615ULL;
    for (auto sample : mSamples)
    {
        std::uint64_t distance = sample.timestampNs - timestampNs;
        if (distance < bestDistance)
        {
            best = sample;
            bestDistance = distance;
        }
    }
    if (best && bestDistance <= static_cast<std::uint64_t>(tolerance.count()))
    {
        return best;
    }
    return std::nullopt;
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

std::optional<Odometry> OdometryBuffer::interpolate(
    [[maybe_unused]] std::uint64_t timestampNs, [[maybe_unused]] std::chrono::nanoseconds maxGap) const
{
    return std::nullopt;
}

std::uint64_t OdometryBuffer::droppedCount() const
{
    return mDroppedCount;
}

} // namespace sensorSync
