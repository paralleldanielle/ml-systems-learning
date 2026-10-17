#include "sensorSync/latencyStats.h"

namespace sensorSync
{

// ---- Session 2 -------------------------------------------------------------------------------------------------

void LatencyStats::record([[maybe_unused]] std::chrono::nanoseconds value)
{
    // TODO(session 2)
}

std::size_t LatencyStats::count() const
{
    // TODO(session 2)
    return 0;
}

std::optional<std::chrono::nanoseconds> LatencyStats::percentile([[maybe_unused]] double percent) const
{
    // TODO(session 2)
    return std::nullopt;
}

std::optional<std::chrono::nanoseconds> LatencyStats::max() const
{
    // TODO(session 2)
    return std::nullopt;
}

} // namespace sensorSync
