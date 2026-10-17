# Teacher notes: 01-sensor-sync

Student: please don't read this folder. It holds the hints and the answers.

## Intent

First contact with modern C++ after a long break, wrapped in a problem the student already understands from the vehicle: time alignment. The point is the *language* mechanics, not the systems idea:

- value types and `const&`
- `std::deque` as a FIFO time series
- `std::optional` for "no answer"
- `std::chrono` units
- the unsigned arithmetic trap
- `double` precision versus `uint64_t` nanoseconds
- angle wrap-around

The tests are the spec, and the replay makes the result tangible.

## Common pitfalls (expect these; they are good review material)

| Pitfall | Rule | Symptom |
|---|---|---|
| `sample.timestampNs - timestampNs` as a distance | CPP-TYP-2 | `QueryAfterNewestSample`, `PicksCloserOfTwoSamples` fail: the distance is ~1.8e19 |
| Comparing timestamps through `double` | CPP-TYP-2, RT-3 | `NanosecondResolutionAtEpochTimestamps` and all interpolation position tests fail by about 1e-6 m |
| `tolerance.count()` compared with ms/ns confusion, or a signed/unsigned comparison warning | RT-3, CPP-ERR-3 | `-Werror` build failure from `-Wsign-compare`, or a wrong inclusive boundary |
| `<` instead of `<=` on tolerance; `<=` instead of `<` on the tie | API contract | `ToleranceIsInclusive` / `TieGoesToEarlierSample` |
| `for (auto sample : mSamples)` copies every sample | CPP-CON-2 | Works, but a review point (minor) |
| Keeping an iterator or reference across `pop_front` | CPP-OWN-3 | ASan heap-use-after-free |
| Eviction implemented before the order check | Contract | `RejectedSampleDoesNotEvict` |
| Naive yaw lerp `a + f*(b-a)` | wrap-around | `YawTakesShortestArcAcrossPi`; replay yaw max ~180° |
| Normalising to [-π, π) instead of (-π, π] | Contract | `YawResultIsNormalised` only near ±π |
| `percent / 100 * n` floating error, off-by-one rank | — | `NearestRankOnHundredValuesInRandomOrder` at p99 |
| Sorting `mValues` inside a `const` method | CPP-CON-1 | Compile error, which is a great teaching moment about `const` and copies |

## Hint ladders

### `add()`
- L1: "What is the only sample you need to look at to know whether a new one is in order?"
- L2: `std::deque::back()`, `empty()`, `push_back()`. Think about what `back()` does on an empty deque (undefined behaviour).
- L3: if not empty and new <= newest → count, return false; otherwise push, return true.
- L4: `if (!mSamples.empty() && sample.timestampNs <= mSamples.back().timestampNs)`

### `findNearest()`
- L1: "How would you compute 'how far apart' two timestamps are, if either one can be the larger?" Then: "What does the tolerance type tell you about its unit?"
- L2: unsigned subtraction wraps around (cppreference: arithmetic operators, unsigned overflow). `std::chrono::nanoseconds::count()` is signed `int64`. A range-for over `mSamples` by `const&` works.
- L3: track the best-so-far as `std::optional<Odometry>` plus its distance; replace it only on a strictly smaller distance (that gives the tie rule); return the best if its distance is <= tolerance.
- L4: `std::uint64_t const d = a > b ? a - b : b - a;`

### `discardBefore()`
- L1: "Where in the deque are the old samples?"
- L2: `front()`, `pop_front()`, and a `while` loop with an emptiness check first.
- L3: while not empty and front is older than t → pop.

### `interpolate()`
- L1: "Which two samples bracket the query? What should happen exactly on a sample?"
- L2: `std::lower_bound` with a comparator (cppreference has the example). Subtract timestamps *as integers* before converting to `double`. Wrap an angle with `std::remainder(a, 2π)`.
- L3: check range → lower_bound → exact hit? return it → `before = prev(it)` → gap check → fraction from integer differences → lerp x, y → yaw: `wrap(before + f * wrap(after - before))`.
- L4: `std::remainder(angle, 2.0 * kPi)` returns a value in [-π, π]; map -π to π.

### `LatencyStats`
- L1: "Nearest rank: which position in the sorted list is the p-th percentile? Try n = 4, p = 50 by hand."
- L2: `std::nth_element` (O(n)) or sort a copy. `std::ceil`. Multiply before dividing.
- L3: rank = ceil(p·n/100), clamp to [1, n], index = rank - 1, nth_element on a copy.

## Recall question bank (pick 3–5)

- What does `std::optional<Odometry>` tell a caller that a returned `Odometry{}` would not?
- `std::uint64_t a = 5, b = 7;` What is `a - b`? How do you get |a - b| safely?
- Why might comparing two `uint64_t` epoch-nanosecond timestamps through `double` give the wrong answer?
- What does `const` after a member function's parameter list promise?
- Which container operation is O(1) at both ends: `std::vector` or `std::deque`?
- What is the shortest angular distance from 179° to -179°?
- When is a reference to a deque element invalidated?
- What is the difference between p99 and max, and which would you put on a dashboard for a 10 Hz control loop?

## Review drill seeds (for `/teacher drill`)

Write a plausible "agent-written" `findNearest` or `interpolate` with 2–3 of these flaws:

- unsigned distance (CPP-TYP-2)
- a by-value range-for copy (CPP-CON-2)
- a magic `5000000` tolerance (CPP-STY-3)
- returning `Odometry{}` instead of `std::nullopt` (CPP-API-1)
- naive yaw lerp
- `front()` on an empty deque (CPP-OWN-3 / UB)
- an `int` loop counter compared with `size()`

Ask the student to find them before revealing.

## Reference solution

`.teacher/reference/` is built with `cmake --preset reference && cmake --build --preset reference && ctest --preset reference`. Only show it after an explicit, confirmed reveal.
