# Exercise 01: Sensor Synchronizer

> **Scenario.** The perception team reports that 3D boxes "lag behind" the car, badly in turns and occasionally by a lot. Every camera frame needs the vehicle pose *at the moment the frame was captured*. Odometry comes in at 50 Hz, camera frames at 10 Hz, and both stamp time in nanoseconds on the vehicle's monotonic clock. Messages jitter, get dropped, and now and then arrive out of order.

You will build the piece that answers "where was the car when this frame was taken?", then measure how good the answer is.

## What is provided

| Path | What it is | Do you edit it? |
|---|---|---|
| `include/sensorSync/*.h` | Types and the class contracts (read these first: they are the spec) | Rarely. Private members are yours to change. |
| `src/*.cpp` | Stubs marked `TODO(session N)` | **Yes**, this is your work |
| `tests/odometryBufferSession1Test.cpp`, `...Session2Test.cpp`, `latencyStatsTest.cpp` | Provided tests per session | No |
| `tests/student_*.cpp` | Your own tests | **Yes**, add at least one per session |
| `app/replay.cpp` | Simulated drive around a circle that prints a match-quality report | No (read it if you're curious) |

Commands (from the repo root):

```bash
cmake --preset dev                                  # once
cmake --build --preset dev                          # after every change
ctest --preset dev -L ex01-s1                       # session-1 tests
./build/dev/exercises/01-sensor-sync/ex01_replay    # the report
```

## Session 1: Nearest match

**Recall first (no notes, 2 minutes each).** What does `std::optional<T>` express that a sentinel value like `-1` does not? What is `5u - 7u` for `std::uint64_t`? When would you take a parameter by `T const&` rather than by value?

**Implement** in `src/odometryBuffer.cpp`:

1. `add()`: store in-order samples, and reject (and count) out-of-order or duplicate ones.
2. `findNearest()`: the closest sample within a tolerance, with the earlier sample winning a tie.
3. `discardBefore()`: drop old history so memory stays bounded.

**Done when** `ctest --preset dev -L ex01-s1` is green with no sanitizer reports, you added one test of your own, and the replay's `nearest` row shows a match rate above 90 %.

**Look at the replay output.** The nearest match is up to ~10 ms off. How far does a car at 10 m/s travel in that time? Is that acceptable for a box at 40 m?

## Session 2: Interpolate and account

**Recall first.** What did your last review flag? How do you represent "a duration in nanoseconds" so the compiler stops you from mixing it with milliseconds? Why can a `double` not hold every `uint64_t`?

**Implement:**

1. `add()`: when the buffer is full, evict the oldest sample and count it (a drop-oldest policy).
2. `interpolate()`: a linear pose between the two samples that bracket the query. Heading must take the shortest way around the circle. (Watch the replay's `yaw max` column: the car crosses ±π twice per run.)
3. `LatencyStats` in `src/latencyStats.cpp`: `record`, `count`, a nearest-rank `percentile` and `max`.

**Done when** `ctest --preset dev -L ex01` is green, you added one test, and the replay's `interpolate` row has a heading error max well under 1°.

## Stretch (optional extra session)

Replace the linear scan in `findNearest()` with `std::lower_bound` (the samples are sorted, so why scan?), then measure both versions with the `release` preset on a buffer of 10 000 samples. Report p50/p99, not just a mean (see TST-4).

## Rules in play

From `docs/engineering-practices.md`, introduced this exercise: RT-1…RT-8, CPP-STY-*, CPP-CON-1/2, CPP-TYP-*, CPP-API-*, CPP-ERR-1/3, CPP-OWN-1/3/4, CPP-STL-*, TST-1/2/3/5. Watch especially for **CPP-TYP-2** (unsigned differences) and **RT-3** (units).
