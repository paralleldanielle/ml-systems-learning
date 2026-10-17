// Replay driver: simulates a car driving a circle (so heading crosses the +pi/-pi seam) with a 10 Hz camera and
// 50 Hz odometry, feeds your OdometryBuffer, and prints how well camera frames get matched to poses.
//
// Run: ./build/dev/exercises/01-sensor-sync/ex01_replay [--seed N]
//
// This file is provided; you do not need to change it. It only uses the public API from include/sensorSync.

#include "sensorSync/latencyStats.h"
#include "sensorSync/odometryBuffer.h"
#include "sensorSync/sensorTypes.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <queue>
#include <random>
#include <string>
#include <vector>

namespace
{

using namespace std::chrono_literals;
using sensorSync::CameraFrame;
using sensorSync::LatencyStats;
using sensorSync::Odometry;
using sensorSync::OdometryBuffer;

constexpr double kPi = 3.14159265358979323846;

// Scenario.
constexpr std::uint64_t kStartNs = 1'700'000'000'000'000'000ULL;
constexpr auto kDuration = 30s;
constexpr auto kOdometryPeriod = 20ms;      // 50 Hz
constexpr auto kCameraPeriod = 100ms;       // 10 Hz
constexpr auto kOdometryJitterStdDev = 1ms; // measurement timestamp jitter
constexpr auto kOdometryTransportDelay = 2ms;
constexpr auto kCameraTransportDelay = 35ms; // exposure + ISP + transport before the frame reaches us
constexpr auto kCameraTransportJitter = 8ms; // extra, uniformly distributed
constexpr auto kCameraPhase = 7ms;           // camera triggers are not aligned with odometry samples
constexpr double kOdometryDropProbability = 0.03;
constexpr double kOdometryReorderProbability = 0.01;
constexpr double kCircleRadiusMeters = 20.0;
constexpr double kSpeedMetersPerSecond = 10.0;

// Synchroniser settings.
constexpr std::size_t kBufferCapacity = 64; // > (camera delay + history) / odometry period
constexpr auto kNearestTolerance = 15ms;
constexpr auto kInterpolationMaxGap = 60ms;
constexpr auto kHistoryToKeep = 500ms;

enum class SourceKind : std::uint8_t
{
    kCamera,
    kOdometry
};

//! A message as it arrives at the synchroniser, ordered by arrival time.
struct Arrival
{
    std::uint64_t arrivalNs{0};
    SourceKind kind{SourceKind::kCamera};
    CameraFrame frame;
    Odometry odometry;
};

struct LaterArrival
{
    bool operator()(Arrival const& a, Arrival const& b) const
    {
        return a.arrivalNs > b.arrivalNs;
    }
};

double wrapAngle(double angleRad)
{
    double wrapped = std::remainder(angleRad, 2.0 * kPi);
    if (wrapped <= -kPi)
    {
        wrapped += 2.0 * kPi;
    }
    return wrapped;
}

//! Ground-truth pose of the car at a given time.
Odometry truePose(std::uint64_t timestampNs)
{
    // Signed difference: jitter can put a sample slightly before kStartNs.
    double const seconds = static_cast<double>(static_cast<std::int64_t>(timestampNs - kStartNs)) * 1e-9;
    double const angleRad = kSpeedMetersPerSecond / kCircleRadiusMeters * seconds;
    Odometry pose;
    pose.timestampNs = timestampNs;
    pose.xMeters = kCircleRadiusMeters * std::cos(angleRad);
    pose.yMeters = kCircleRadiusMeters * std::sin(angleRad);
    pose.yawRad = wrapAngle(angleRad + kPi / 2.0);
    return pose;
}

std::uint64_t offsetNs(std::uint64_t base, std::chrono::nanoseconds offset)
{
    return static_cast<std::uint64_t>(static_cast<std::int64_t>(base) + offset.count());
}

std::priority_queue<Arrival, std::vector<Arrival>, LaterArrival> simulateArrivals(std::uint32_t seed)
{
    std::mt19937 generator{seed};
    std::normal_distribution<double> jitterNs{
        0.0, static_cast<double>(std::chrono::nanoseconds{kOdometryJitterStdDev}.count())};
    std::uniform_int_distribution<std::int64_t> cameraJitterNs{
        0, std::chrono::nanoseconds{kCameraTransportJitter}.count()};
    std::uniform_real_distribution<double> unit{0.0, 1.0};

    std::priority_queue<Arrival, std::vector<Arrival>, LaterArrival> arrivals;
    std::uint64_t const endNs = offsetNs(kStartNs, kDuration);

    std::uint64_t previousOdometryArrivalNs = 0;
    for (std::uint64_t t = kStartNs; t < endNs; t = offsetNs(t, kOdometryPeriod))
    {
        if (unit(generator) < kOdometryDropProbability)
        {
            continue;
        }
        auto const jitter = std::chrono::nanoseconds{static_cast<std::int64_t>(jitterNs(generator))};
        Arrival arrival;
        arrival.kind = SourceKind::kOdometry;
        arrival.odometry = truePose(offsetNs(t, jitter));
        arrival.arrivalNs = offsetNs(arrival.odometry.timestampNs, kOdometryTransportDelay);
        // Occasionally a message overtakes its predecessor on the bus.
        if (previousOdometryArrivalNs != 0 && unit(generator) < kOdometryReorderProbability)
        {
            arrival.arrivalNs = previousOdometryArrivalNs - 1;
        }
        previousOdometryArrivalNs = arrival.arrivalNs;
        arrivals.push(arrival);
    }

    for (std::uint64_t t = offsetNs(kStartNs, kCameraPhase); t < endNs; t = offsetNs(t, kCameraPeriod))
    {
        Arrival arrival;
        arrival.kind = SourceKind::kCamera;
        arrival.frame.timestampNs = t;
        auto const transportJitter = std::chrono::nanoseconds{cameraJitterNs(generator)};
        arrival.arrivalNs = offsetNs(t, kCameraTransportDelay + transportJitter);
        arrivals.push(arrival);
    }
    return arrivals;
}

struct MatchErrors
{
    LatencyStats timeOffset; //!< |frame time - matched odometry time|
    std::vector<double> positionErrorMeters;
    std::vector<double> yawErrorDeg;
    std::size_t matched{0};
};

void recordMatch(MatchErrors& errors, std::uint64_t frameNs, std::optional<Odometry> const& pose)
{
    if (!pose)
    {
        return;
    }
    ++errors.matched;
    Odometry const truth = truePose(frameNs);
    std::int64_t const offset = frameNs > pose->timestampNs ? static_cast<std::int64_t>(frameNs - pose->timestampNs)
                                                            : static_cast<std::int64_t>(pose->timestampNs - frameNs);
    errors.timeOffset.record(std::chrono::nanoseconds{offset});
    errors.positionErrorMeters.push_back(std::hypot(pose->xMeters - truth.xMeters, pose->yMeters - truth.yMeters));
    errors.yawErrorDeg.push_back(std::abs(wrapAngle(pose->yawRad - truth.yawRad)) * 180.0 / kPi);
}

double percentileOf(std::vector<double> values, double percent)
{
    if (values.empty())
    {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    auto const rank = static_cast<std::size_t>(std::ceil(percent * static_cast<double>(values.size()) / 100.0));
    return values[std::max<std::size_t>(rank, 1) - 1];
}

std::string formatMs(std::optional<std::chrono::nanoseconds> value)
{
    if (!value)
    {
        return "   n/a";
    }
    char text[32];
    std::snprintf(text, sizeof(text), "%6.2f", std::chrono::duration<double, std::milli>(*value).count());
    return text;
}

void printMatchRow(char const* name, MatchErrors const& errors, std::size_t frames)
{
    double const ratePercent
        = frames == 0 ? 0.0 : 100.0 * static_cast<double>(errors.matched) / static_cast<double>(frames);
    std::printf("  %-12s %5.1f %%   dt p50/p99 %s/%s ms   pos p50/max %6.3f/%6.3f m   yaw p50/max %7.2f/%7.2f deg\n",
        name, ratePercent, formatMs(errors.timeOffset.percentile(50.0)).c_str(),
        formatMs(errors.timeOffset.percentile(99.0)).c_str(), percentileOf(errors.positionErrorMeters, 50.0),
        percentileOf(errors.positionErrorMeters, 100.0), percentileOf(errors.yawErrorDeg, 50.0),
        percentileOf(errors.yawErrorDeg, 100.0));
}

} // namespace

int main(int argc, char** argv)
{
    std::uint32_t seed = 1;
    if (argc == 3 && std::string{argv[1]} == "--seed")
    {
        seed = static_cast<std::uint32_t>(std::strtoul(argv[2], nullptr, 10));
    }

    auto arrivals = simulateArrivals(seed);
    OdometryBuffer buffer{kBufferCapacity};
    MatchErrors nearestErrors;
    MatchErrors interpolatedErrors;
    LatencyStats cameraInterArrival;
    std::uint64_t previousCameraArrivalNs = 0;
    std::size_t frames = 0;
    std::size_t odometryMessages = 0;
    std::size_t maxBufferSize = 0;

    while (!arrivals.empty())
    {
        Arrival const arrival = arrivals.top();
        arrivals.pop();
        switch (arrival.kind)
        {
        case SourceKind::kOdometry:
            ++odometryMessages;
            buffer.add(arrival.odometry);
            break;
        case SourceKind::kCamera:
        {
            ++frames;
            std::uint64_t const frameNs = arrival.frame.timestampNs;
            if (previousCameraArrivalNs != 0)
            {
                cameraInterArrival.record(std::chrono::nanoseconds{arrival.arrivalNs - previousCameraArrivalNs});
            }
            previousCameraArrivalNs = arrival.arrivalNs;
            recordMatch(nearestErrors, frameNs, buffer.findNearest(frameNs, kNearestTolerance));
            recordMatch(interpolatedErrors, frameNs, buffer.interpolate(frameNs, kInterpolationMaxGap));
            auto const historyNs = static_cast<std::uint64_t>(std::chrono::nanoseconds{kHistoryToKeep}.count());
            if (frameNs > kStartNs + historyNs)
            {
                buffer.discardBefore(frameNs - historyNs);
            }
            break;
        }
        }
        maxBufferSize = std::max(maxBufferSize, buffer.size());
    }

    std::printf(
        "Sensor sync replay (seed %u): %zu camera frames, %zu odometry messages\n", seed, frames, odometryMessages);
    std::printf("  odometry rejected (out of order) %llu, dropped (buffer full) %llu, max buffer size %zu/%zu\n",
        static_cast<unsigned long long>(buffer.rejectedCount()), static_cast<unsigned long long>(buffer.droppedCount()),
        maxBufferSize, kBufferCapacity);
    std::printf("  camera inter-arrival p50/p99/max %s/%s/%s ms\n",
        formatMs(cameraInterArrival.percentile(50.0)).c_str(), formatMs(cameraInterArrival.percentile(99.0)).c_str(),
        formatMs(cameraInterArrival.max()).c_str());
    std::printf("  method       matched   time offset            position error               heading error\n");
    printMatchRow("nearest", nearestErrors, frames);
    printMatchRow("interpolate", interpolatedErrors, frames);
    return EXIT_SUCCESS;
}
