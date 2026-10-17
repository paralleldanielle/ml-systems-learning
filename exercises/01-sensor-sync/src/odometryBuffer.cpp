#include "sensorSync/odometryBuffer.h"

#include <cassert>

namespace sensorSync
{

OdometryBuffer::OdometryBuffer(std::size_t capacity) : mCapacity{capacity}
{
    assert(capacity > 0 && "OdometryBuffer capacity must be greater than zero");
}

// ---- Session 1 -------------------------------------------------------------------------------------------------

bool OdometryBuffer::add([[maybe_unused]] Odometry const& sample)
{
    // TODO(session 1): reject out-of-order and duplicate samples (and count them), otherwise store the sample.
    // TODO(session 2): when the buffer is full, evict the oldest sample first (and count it).
    return false;
}

std::optional<Odometry> OdometryBuffer::findNearest(
    [[maybe_unused]] std::uint64_t timestampNs, [[maybe_unused]] std::chrono::nanoseconds tolerance) const
{
    // TODO(session 1)
    return std::nullopt;
}

void OdometryBuffer::discardBefore([[maybe_unused]] std::uint64_t timestampNs)
{
    // TODO(session 1)
}

std::size_t OdometryBuffer::size() const
{
    return mSamples.size();
}

std::uint64_t OdometryBuffer::rejectedCount() const
{
    return mRejectedCount;
}

// ---- Session 2 -------------------------------------------------------------------------------------------------

std::optional<Odometry> OdometryBuffer::interpolate(
    [[maybe_unused]] std::uint64_t timestampNs, [[maybe_unused]] std::chrono::nanoseconds maxGap) const
{
    // TODO(session 2)
    return std::nullopt;
}

std::uint64_t OdometryBuffer::droppedCount() const
{
    return mDroppedCount;
}

} // namespace sensorSync
