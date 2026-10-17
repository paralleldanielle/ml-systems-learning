#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <vector>

namespace sensorSync
{

//! Collects duration samples (latencies, inter-arrival intervals, ...) and reports their distribution.
//! Session 2.
class LatencyStats
{
public:
    //! Record one duration. Negative durations are allowed (e.g. a clock offset) and are stored as-is.
    void record(std::chrono::nanoseconds value);

    //! Number of recorded values.
    std::size_t count() const;

    //! Nearest-rank percentile: the smallest recorded value such that at least `percent` % of all values are
    //! less than or equal to it. Precondition: 0 < percent <= 100.
    //! \return std::nullopt if nothing has been recorded.
    std::optional<std::chrono::nanoseconds> percentile(double percent) const;

    //! Largest recorded value, or std::nullopt if nothing has been recorded.
    std::optional<std::chrono::nanoseconds> max() const;

private:
    std::vector<std::chrono::nanoseconds> mValues;
};

} // namespace sensorSync
