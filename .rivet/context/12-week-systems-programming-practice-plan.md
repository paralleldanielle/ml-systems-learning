# 12-Week Systems & Programming Practice Plan

## Goal

Build stronger hands-on C++ and Python fluency through small, realistic exercises based on ML, sensor, robotics, and vehicle-integration problems.

The goal is **not** to become a generic competitive programmer. It is to develop enough programming muscle memory to:

- reason about unfamiliar code without an assistant
- write clean, idiomatic C++ and Python
- recognize ownership, concurrency, latency, and performance problems
- review AI-generated code critically
- debug systems independently
- become more confident in technical interviews

The exercises should be small enough to fit into a protected weekly focus block.

---

## Working Rules

### 1. First attempt: no AI-generated solution

Use:

- compiler/interpreter
- official documentation
- debugger
- profiling tools
- web documentation

Do **not** have an AI agent write the solution on the first attempt.

If stuck for ~20 minutes, ask for a **hint**, not the solution.

### 2. After solving: use AI as a reviewer

Give the finished implementation to an agent and ask it to review for:

- correctness
- ownership/lifetime
- concurrency
- API design
- error handling
- performance
- testing
- maintainability

Ask it to identify problems rather than rewrite the code.

### 3. Keep exercises realistic

Prefer problems involving:

- sensor data
- timestamps
- queues
- pipelines
- inference
- trajectories
- latency
- concurrency
- CPU/GPU boundaries
- robotics/vehicle systems

Avoid spending the limited practice time on generic puzzles unless they teach a directly useful concept.

---

# 12-Week Plan

## Weeks 1–2 — Sensor Synchronizer

### Build

A small C++ program that receives:

- camera frames at 10 Hz
- odometry at 50 Hz
- timestamps in nanoseconds

Example data types:

```cpp
struct CameraFrame {
    uint64_t timestamp_ns;
    std::vector<uint8_t> data;
};

struct Odometry {
    uint64_t timestamp_ns;
    double x;
    double y;
    double yaw;
};
```

Implement:

- message queues
- timestamp matching
- stale-message dropping
- timeout handling
- interpolation
- latency/jitter/drop statistics

### Concepts

- structs/classes
- STL containers
- references
- `std::optional`
- time representations
- API design
- testing

### Stretch

Add configurable synchronization tolerance and benchmark different lookup strategies.

---

## Weeks 3–4 — Multithreaded Pipeline

Extend the synchronizer into:

```text
Camera thread
      ↓
Synchronizer thread
      ↓
Inference thread
      ↓
Planner thread
```

Implement:

- producer/consumer queues
- bounded queues
- mutexes
- condition variables
- graceful shutdown

Then deliberately introduce and diagnose:

- a race condition
- a deadlock
- an unbounded queue
- backpressure
- frame drops
- slow inference

### Concepts

- `std::thread`
- `std::mutex`
- `std::condition_variable`
- atomics
- synchronization
- ownership
- thread lifetime
- concurrency debugging

---

## Weeks 5–6 — Inference Scheduler

Simulate inference with variable latency:

```text
Typical: 80 ms
Occasional: 150 ms
Worst case: 900 ms
Input: 10 Hz
```

Implement and compare:

1. Process every frame
2. Always process newest frame
3. Queue everything
4. Bounded queue + drop oldest
5. Bounded queue + drop newest

Measure:

- throughput
- end-to-end latency
- input staleness
- dropped frames
- queue depth

### Engineering question

Which scheduling policy is best for a closed-loop system, and why?

Document the tradeoffs.

### Concepts

- latency vs throughput
- bounded buffering
- real-time-ish constraints
- backpressure
- benchmarking
- measurement-driven decisions

---

## Weeks 7–8 — Trajectory Buffer

Implement:

```cpp
class TrajectoryBuffer {
public:
    void add(TrajectoryPoint point);
    std::optional<TrajectoryPoint> at(Time timestamp) const;
    void discard_before(Time timestamp);
};
```

Develop it progressively:

1. Linear search
2. Binary search
3. Interpolation
4. Bounded memory
5. Thread safety
6. Benchmarking

### Concepts

- class design
- `const`
- STL algorithms
- complexity
- ownership
- API contracts
- performance measurement

### Python companion

Implement the same concept in Python using:

- `dataclass`
- type hints
- pytest
- clean API design

Compare the two implementations.

---

## Weeks 9–10 — Code Review & Debugging

Take a deliberately flawed C++ program containing several issues, such as:

- unnecessary `shared_ptr`
- dangling reference
- data race
- unnecessary large-vector copy
- blocking operation inside a callback
- missing `const`
- swallowed exception
- accidental quadratic algorithm

### Task

Do **not** rewrite it immediately.

First produce a review:

```text
Issue
Why it matters
How to reproduce/observe it
Proposed fix
Tradeoff
```

Then fix the issues individually.

### Goal

Develop the ability to look at code and quickly recognize:

- bad ownership
- lifetime problems
- unnecessary copies
- concurrency hazards
- poor APIs
- performance traps

This is especially useful when reviewing AI-generated code.

---

# Weeks 11–12 — CPU/GPU Pipeline

Start with a simple CPU implementation, such as:

> Convert a 1080×1920 RGB image to grayscale.

Implement and benchmark:

1. Simple CPU implementation
2. Improved/optimized CPU implementation
3. CUDA implementation

Investigate:

- host/device memory transfers
- kernel launch overhead
- memory access
- synchronization
- batch size
- utilization

Use profiling tools to determine where the time actually goes.

### Goal

Not to become a CUDA specialist.

Instead, learn to reason about:

- CPU/GPU boundaries
- data movement
- kernel execution
- latency
- throughput
- profiling results

---

# Capstone — Miniature Vehicle Pipeline

If time allows, combine the earlier exercises into a small simulated system.

## Scenario

Four cameras produce data at 10 Hz.

Odometry arrives at 50 Hz.

Camera and vehicle clocks have an offset.

Inference takes 80–150 ms with occasional long-tail latency.

The downstream planner expects synchronized observations.

Build:

```text
Cameras ──┐
          ├──> Synchronizer ──> Inference ──> Trajectory ──> Planner
Odometry ─┘
```

Then deliberately introduce failures.

Examples:

- timestamp offset
- stale frames
- slow inference
- queue overflow
- dropped messages
- race condition
- clock-domain mismatch

Diagnose and fix them.

---

# One-Page Engineering Report

For the capstone, write a short engineering report:

## Problem

What problem are we solving?

## Assumptions

What are we assuming about timing, rates, latency, and data?

## Architecture

What components exist and why?

## Failure Modes

What can go wrong?

## Measurements

What did we actually measure?

## Decision

What implementation did we choose?

## Tradeoffs

What did we optimize for, and what did we sacrifice?

---

# Weekly Format

A single protected focus block is enough.

Suggested format:

### 90 minutes

**10 min — Review**

Recall the relevant concepts without looking them up.

**50 min — Implementation**

Write the exercise yourself.

**20 min — Debug/test/benchmark**

Make it work and measure it.

**10 min — Reflection**

Write down:

- What did I learn?
- What did I forget?
- What felt difficult?
- What would I recognize faster next time?

The goal is repeated retrieval, not memorizing a textbook.

---

# Priority Order

If work gets busy, keep the exercises in this order:

1. Sensor synchronizer
2. Multithreaded pipeline
3. Inference scheduler
4. Trajectory buffer
5. Debugging/code review
6. CPU/GPU pipeline

Do not try to complete everything perfectly.

Consistency matters more than completion.

---

# Success Criteria

After three months, the target is not:

> "I know C++."

The target is:

> "I can sit down with a moderately unfamiliar C++ systems problem and reason my way through it."

Specifically, I want to be able to:

- write basic modern C++ without constantly looking up syntax
- reason about ownership and lifetime
- recognize common concurrency problems
- design simple APIs
- use STL effectively
- write and run tests
- benchmark before optimizing
- read unfamiliar C++ with confidence
- explain Python design choices
- debug systematically
- critically review AI-generated code
- reason about CPU/GPU performance

That is the skill set to build.
