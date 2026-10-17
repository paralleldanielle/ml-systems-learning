#pragma once

#include "sensorSync/sensorTypes.h"

#include <chrono>
#include <cmath>
#include <cstdint>

namespace sensorSync::test
{

//! A realistic absolute timestamp (about 2023 in Unix-epoch nanoseconds). Deliberately not a round number: values
//! this large cannot be represented exactly as a double, which is part of what the tests check.
constexpr std::uint64_t kT0 = 1'700'000'000'123'456'789ULL;

constexpr double kPi = 3.14159265358979323846;

//! Nanosecond offset from kT0.
constexpr std::uint64_t at(std::chrono::nanoseconds offset)
{
    return kT0 + static_cast<std::uint64_t>(offset.count());
}

inline Odometry makeOdometry(std::uint64_t timestampNs, double xMeters = 0.0, double yMeters = 0.0, double yawRad = 0.0)
{
    Odometry odometry;
    odometry.timestampNs = timestampNs;
    odometry.xMeters = xMeters;
    odometry.yMeters = yMeters;
    odometry.yawRad = yawRad;
    return odometry;
}

//! Smallest signed difference a - b between two angles, in (-pi, pi].
inline double angleDifference(double aRad, double bRad)
{
    double difference = std::remainder(aRad - bRad, 2.0 * kPi);
    if (difference <= -kPi)
    {
        difference += 2.0 * kPi;
    }
    return difference;
}

} // namespace sensorSync::test
