#pragma once

#include "sensorSync/sensorTypes.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>

namespace sensorSync
{

//! A time-ordered, bounded history of odometry samples that camera frames can be matched against.
//!
//! Contract:
//! - Samples are stored in strictly increasing timestamp order.
//! - At most `capacity` samples are kept; when full, the oldest sample is evicted (drop-oldest policy).
//! - All timestamps are nanoseconds on the vehicle's monotonic clock.
class OdometryBuffer
{
public:
    //! \param capacity Maximum number of samples kept. Must be greater than zero.
    explicit OdometryBuffer(std::size_t capacity);

    // ---- Session 1 -------------------------------------------------------------------------------------------

    //! Append a sample.
    //! A sample whose timestamp is not strictly newer than the newest stored sample (out of order, or a
    //! duplicate) is rejected and counted in rejectedCount().
    //! \return true if the sample was stored.
    bool add(Odometry const& sample);

    //! The stored sample closest in time to `timestampNs`, if its distance is within `tolerance` (inclusive).
    //! If two samples are equally close, the earlier one is returned.
    //! \return std::nullopt if the buffer is empty or no sample is within tolerance.
    std::optional<Odometry> findNearest(std::uint64_t timestampNs, std::chrono::nanoseconds tolerance) const;

    //! Remove every sample strictly older than `timestampNs`. A sample exactly at `timestampNs` is kept.
    void discardBefore(std::uint64_t timestampNs);

    //! Number of samples currently stored.
    std::size_t size() const;

    //! Number of samples rejected by add() because they were out of order or duplicates.
    std::uint64_t rejectedCount() const;

    // ---- Session 2 -------------------------------------------------------------------------------------------

    //! The pose at `timestampNs`, linearly interpolated between the two stored samples that bracket it.
    //! - Position is interpolated linearly.
    //! - Yaw is interpolated along the shortest arc and the result is normalised to (-pi, pi].
    //! - The returned sample's timestampNs equals `timestampNs`.
    //! - If `timestampNs` equals a stored sample's timestamp, that sample is returned unchanged.
    //! \return std::nullopt if `timestampNs` is outside [oldest, newest] or the bracketing samples are more than
    //!         `maxGap` apart (too much missing data to trust a straight line).
    std::optional<Odometry> interpolate(std::uint64_t timestampNs, std::chrono::nanoseconds maxGap) const;

    //! Number of samples evicted because the buffer was full.
    std::uint64_t droppedCount() const;

private:
    std::size_t mCapacity;
    std::deque<Odometry> mSamples;
    std::uint64_t mRejectedCount{0};
    std::uint64_t mDroppedCount{0};
};

} // namespace sensorSync
