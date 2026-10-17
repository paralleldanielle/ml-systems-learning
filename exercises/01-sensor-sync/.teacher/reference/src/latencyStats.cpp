#include "sensorSync/latencyStats.h"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace sensorSync
{

void LatencyStats::record(std::chrono::nanoseconds value)
{
    mValues.push_back(value);
}

std::size_t LatencyStats::count() const
{
    return mValues.size();
}

std::optional<std::chrono::nanoseconds> LatencyStats::percentile(double percent) const
{
    assert(percent > 0.0 && percent <= 100.0 && "percentile must be in (0, 100]");
    if (mValues.empty())
    {
        return std::nullopt;
    }

    std::size_t const n = mValues.size();
    // Nearest rank (1-based); multiply before dividing so that e.g. 99 % of 100 is exactly 99.
    auto const rank = static_cast<std::size_t>(std::ceil(percent * static_cast<double>(n) / 100.0));
    std::size_t const index = std::clamp<std::size_t>(rank, 1, n) - 1;

    // Copy so that the query stays const; statistics are read off the hot path.
    std::vector<std::chrono::nanoseconds> sorted = mValues;
    auto const nth = sorted.begin() + static_cast<std::ptrdiff_t>(index);
    std::nth_element(sorted.begin(), nth, sorted.end());
    return *nth;
}

std::optional<std::chrono::nanoseconds> LatencyStats::max() const
{
    if (mValues.empty())
    {
        return std::nullopt;
    }
    return *std::max_element(mValues.begin(), mValues.end());
}

} // namespace sensorSync
