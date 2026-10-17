# Review: 01-sensor-sync session 1 — 2026-10-17

Reviewed: eval fixture `flawed-session-1/odometryBuffer.cpp` (+ uncommitted: n/a)
Evidence: build ok (dev, ASan+UBSan, -Werror) · tests 11/16 (ex01-s1) · sanitizers clean · tidy 1 warning (misc-const-correctness) · replay: nearest 95.0 % matched, pos p50 0.130 m (expected ~0.07 m)

*This is a sample output from a dry run of the reviewer skill. It shows the expected shape and depth of a review.*

## What you did well
- **RT-4**: `add()` counts every rejected sample instead of silently ignoring it (`odometryBuffer.cpp:17-21`). That counter is what lets you see a bus problem in a log later.
- **CPP-API-1**: `findNearest()` returns `std::nullopt` rather than an empty `Odometry{}`, so callers cannot accidentally use a zero pose.

## Findings

### 1. [blocker] CPP-TYP-2 — distance wraps around when the query is newer than the sample (`odometryBuffer.cpp:32`)
**In plain terms:** `sample.timestampNs - timestampNs` subtracts two *unsigned* numbers. If the right one is bigger, the result does not go negative; it wraps around to an enormous number (about 1.8 × 10¹⁹). So every sample *older* than the query looks impossibly far away.
**Systems view:** the synchroniser can only ever pick odometry from *after* the frame. In the replay that doubles the typical pose error (0.130 m instead of ~0.07 m). When no newer sample has arrived yet, the frame gets no pose at all, which is the common case for low-latency processing.
**How to see it:** `QueryAfterNewestSample` and `PicksCloserOfTwoSamples` fail. Try it by hand: `std::uint64_t{5} - std::uint64_t{7}`.
**Your move:** how can you compute "how far apart" two unsigned values are without knowing in advance which one is larger?

### 2. [minor] CPP-CON-1 — `distance` never changes but is not `const` (`odometryBuffer.cpp:32`)
**In plain terms:** `distance` is computed once and only read afterwards. Declaring it `std::uint64_t const distance` tells the reader (and the compiler) that it will not change. clang-tidy flags exactly this line.
**Systems view:** in a 300-line callback, knowing at a glance which values are fixed is what makes a code review fast and safe. `const` turns "I hope nobody changes this" into a compile error.
**How to see it:** `cmake --preset tidy && cmake --build --preset tidy` → `misc-const-correctness`.
**Your move:** which other locals in `findNearest` could be `const`?

### 3. [minor] CPP-CON-2 — the loop copies every sample (`odometryBuffer.cpp:30`)
**In plain terms:** `for (auto sample : mSamples)` makes a *copy* of each `Odometry` before looking at it. You only read it, so a `const` reference avoids the copy.
**Systems view:** here it is 32 bytes per sample, which is cheap. But the same habit on a `CameraFrame` (megabytes of pixels) is a per-frame latency regression that no test will catch.
**How to see it:** read the loop header; the copy is what `auto` (without `&`) means.
**Your move:** what changes if you write `auto const&`? What would `auto&` allow that you do not want?

### 4. [minor] CPP-STY-3 — magic number `18446744073709551615ULL` (`odometryBuffer.cpp:29`)
**In plain terms:** this is the largest `uint64_t`, typed out by hand. A reader has to count digits to know that, and a typo would be silent.
**Systems view:** sentinel values that "can never happen" are a classic source of field bugs. You already have `best` (an optional) that says whether you have found anything.
**How to see it:** remove the initial value and ask what `bestDistance` should be before the first sample. Then see whether `best.has_value()` already answers that.
**Your move:** can the comparison use the optional's emptiness instead of a sentinel? (If you really need the maximum, `std::numeric_limits<std::uint64_t>::max()` names it.)

## Preview (not counted)
- **TST-4** (stretch): the samples are sorted, so `std::lower_bound` could replace the full scan. Measure before deciding it matters.

## Recall for next session
- What is `std::uint64_t{5} - std::uint64_t{7}`, and how do you get the absolute difference safely?
- What does `for (auto x : v)` do that `for (auto const& x : v)` does not?
- Why is `std::optional` a better "not found yet" marker than a maximum-value sentinel?
