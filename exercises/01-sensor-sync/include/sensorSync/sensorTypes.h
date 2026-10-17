#pragma once

#include <cstdint>
#include <vector>

namespace sensorSync
{

//! A camera image. The timestamp is the capture time in nanoseconds on the vehicle's monotonic clock.
struct CameraFrame
{
    std::uint64_t timestampNs{0};
    std::vector<std::uint8_t> data;
};

//! A 2D vehicle pose from wheel/IMU odometry. The timestamp is the measurement time in nanoseconds on the
//! vehicle's monotonic clock (the same clock domain as CameraFrame).
struct Odometry
{
    std::uint64_t timestampNs{0};
    double xMeters{0.0};
    double yMeters{0.0};
    double yawRad{0.0}; //!< Heading, normalised to (-pi, pi].
};

} // namespace sensorSync
