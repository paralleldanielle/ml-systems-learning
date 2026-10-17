# Engineering Practices Reference

The coding standard this repository teaches and reviews against. The teacher uses it to choose what to emphasise, and the reviewer cites its rule IDs in every finding.

## Where it comes from

The rules are distilled from public sources used in production GPU and ML-inference software and in automotive functional safety:

- NVIDIA's TensorRT-LLM `CODING_GUIDELINES.md` (C++ and Python), which is derived from the Google C++ Style Guide and PEP 8
- NVIDIA's *CUDA C++ Guidelines for Effective Safety-Critical Programming* (v3.0.1, in the CUDA for Tegra docs), which use MISRA's rule/directive classification
- NVIDIA's *CUDA C++ Best Practices Guide* and its Assess → Parallelize → Optimize → Deploy (APOD) cycle
- ISO 26262-6 (Table 1, the topics coding guidelines must cover), MISRA C++:2023 and AUTOSAR C++14, for the real-time embedded automotive view

The repository is a learning environment, not a certified safety project. The aim is to build the instincts those standards encode. Compliance is not the aim.

## How to use this document

- **Phases.** Each rule names the week it is **introduced**. The reviewer treats rules introduced so far as binding. Later rules may appear only as a short "preview" note and never count against the student.
- **Rule IDs** are stable. Add new rules with new IDs and never renumber existing ones.
- **Tooling first.** If a formatter or compiler flag can enforce a rule, the tooling enforces it, and human review time goes to what tools cannot see.

### Severity scale (shared by teacher and reviewer)

| Severity | Meaning | Example |
|---|---|---|
| **blocker** | Wrong result, undefined behaviour, crash or data race | Unsigned timestamp subtraction underflows |
| **major** | Would be rejected in a real-time vehicle codebase review | Unbounded queue on the per-frame path |
| **minor** | Works, but costs clarity, safety margin or performance | Missing `const` on a non-mutating method |
| **nit** | Taste or formatting; prefer fixing the tooling instead | Comment wording |
| **praise** | Something done well that is worth repeating | Units carried in the type |

---

## RT: Real-time and embedded mindset (applies to every language)

| ID | Rule | Why | Intro |
|---|---|---|---|
| RT-1 | The steady-state hot path (per-message or per-frame code) does bounded work: no unbounded memory growth, no unbounded loops, no blocking I/O. Reserve and allocate at start-up. | Worst-case latency, not average latency, decides whether a control loop is safe. | wk1 |
| RT-2 | Every buffer and queue has an explicit capacity and an explicit overflow policy (drop oldest, drop newest or reject). | Unbounded buffers turn a slow consumer into growing latency and, eventually, memory exhaustion. | wk1 |
| RT-3 | Measure durations and timeouts with a monotonic clock (`std::chrono::steady_clock`, `time.monotonic_ns`). Carry units in the type (`std::chrono::nanoseconds`) or the name (`timestampNs`, `yaw_rad`). Never mix clock domains silently. | Wall clocks jump (NTP, GPS sync). Unit confusion is a classic, expensive bug. | wk1 |
| RT-4 | Fail explicitly and observably. Every drop, timeout or rejected input is counted and exposed, never silently ignored. | You cannot debug a vehicle log for an event that left no trace. | wk1 |
| RT-5 | Validate inputs at module boundaries (out-of-order timestamps, NaN, empty frames) and trust them inside. | Defensive implementation at boundaries; no repeated checks deep inside. | wk1 |
| RT-6 | Keep functions small and simple, with one job per function, shallow nesting and early returns. | Low complexity is "highly recommended" at every ASIL. It is also what makes code reviewable. | wk1 |
| RT-7 | Measure before optimising. Report distributions (p50, p99, max), not just means. | Tail latency causes the incident, and the mean hides it. | wk1 |
| RT-8 | Make the design decision visible: a comment or README line stating the policy chosen (tolerance, drop policy) and why. | A reviewer can only check intent that was written down. | wk1 |

## CPP: Modern C++ (C++17)

### Style and naming (mostly tool-enforced)

| ID | Rule | Why | Intro |
|---|---|---|---|
| CPP-STY-1 | `clang-format` is authoritative: 4-space indent, 120 columns, Allman braces, braces on every `if`/`else`/`for`/`while` body. | No review time spent on layout; braces prevent "goto fail"-style bugs. | wk1 |
| CPP-STY-2 | Types `PascalCase`; functions, methods and variables `camelCase`; private/protected members `mName`; constants `kName`; `enum class` with `kValue` enumerators; macros `UPPER_SNAKE` (avoid them). Public struct fields have no prefix. | Readers can tell scope and mutability from the name alone. | wk1 |
| CPP-STY-3 | No magic numbers: apart from `0`, `nullptr`, `true` and `false`, a literal appears only where a named constant is initialised. | `kSyncToleranceNs` documents intent; `5000000` does not. | wk1 |
| CPP-STY-4 | `//` comments explain *why*, not *what*. Public API gets Doxygen `//!` comments that state units and preconditions. Never leave commented-out code. | The header is the contract, and dead code misleads. | wk1 |
| CPP-STY-5 | `#pragma once`; C++ headers (`<cstdint>`, not `<stdint.h>`); fixed-width integer types where the size matters. | Portability and an explicit data layout. | wk1 |

### Types, const and initialisation

| ID | Rule | Why | Intro |
|---|---|---|---|
| CPP-CON-1 | `const` by default: locals and globals not modified after initialisation are `const`; non-mutating methods are `const`; prefer `constexpr` to `#define`. East-const style (`int const x`). | The compiler then enforces your intent, and readers know what can change. | wk1 |
| CPP-CON-2 | Pass cheap types by value and large read-only types by `T const&`. Sink parameters (ones the callee stores) are taken by value and `std::move`d. | Avoids accidental copies of multi-megabyte frames. | wk1 |
| CPP-TYP-1 | Strong typing: `enum class`, `std::chrono` durations and small structs instead of raw `int`/`bool`/`uint64_t`. Avoid several `bool` parameters in one function. | Mix-ups become compile errors, not field failures. | wk1 |
| CPP-TYP-2 | Be deliberate with signed and unsigned. Never subtract unsigned values that can be "out of order" without checking which is larger first. Convert to a signed duration for differences. | `uint64_t` underflow turns −1 ns into ~584 years. | wk1 |
| CPP-TYP-3 | Use the weakest cast that works: preferably none, then `static_cast`. No C-style casts, never cast away `const`, `reinterpret_cast` only as a last resort. | Casts hide bugs from the compiler. | wk1 |
| CPP-TYP-4 | Initialise every variable at its declaration (brace initialisation). Never `memset` a non-trivial type. | Uninitialised reads are undefined behaviour. | wk1 |

### API design and errors

| ID | Rule | Why | Intro |
|---|---|---|---|
| CPP-API-1 | Make absence explicit: `std::optional<T>` for "may not exist", not sentinel values (`-1`, `0`, an empty struct). | The caller cannot forget to handle the empty case. | wk1 |
| CPP-API-2 | Interfaces are small and hard to misuse. The header states units, preconditions and what happens at the edges (empty buffer, exact tolerance boundary). | An API contract a reviewer can check. | wk1 |
| CPP-ERR-1 | Expected failures (no match, timeout) are return values. Programming errors (broken invariants) are `assert`ions. Exceptions never cross library boundaries, are not used for control flow on the hot path, and are never swallowed. | Predictable control flow and timing. | wk1 |
| CPP-ERR-2 | A `switch` on an `enum class` lists every enumerator and has **no** `default`, so the compiler warns when a value is added. Every case ends in `break`, `return` or `throw`. | New states cannot be silently ignored. | wk3 |
| CPP-ERR-3 | Builds use `-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`. A warning is a bug report, not noise. | The cheapest static analysis available. | wk1 |

### Ownership, lifetime and memory

| ID | Rule | Why | Intro |
|---|---|---|---|
| CPP-OWN-1 | RAII for every resource (memory, locks, threads, files, GPU buffers). No naked `new`/`delete`. | Cleanup happens on every path, including errors. | wk1 |
| CPP-OWN-2 | `std::unique_ptr` for sole ownership, `std::shared_ptr` only for genuinely shared lifetime. Raw pointers and references never own. | Ownership is readable from the type, and `shared_ptr` by default hides design problems. | wk3 |
| CPP-OWN-3 | Never keep a reference, pointer or iterator into a container that may grow, shrink or be popped, and never return a reference to a local or temporary. | Dangling references are silent until they corrupt data. | wk1 |
| CPP-OWN-4 | Large buffers (images, point clouds) are moved or viewed, not copied. `reserve()` capacity up front. | One accidental copy per frame shows up as a large latency regression. | wk1 |
| CPP-OWN-5 | No non-trivial global or static objects. Use a function-local static accessor if one is truly needed. | Avoids the static-initialisation-order fiasco. | wk3 |

### STL and algorithms

| ID | Rule | Why | Intro |
|---|---|---|---|
| CPP-STL-1 | Prefer standard algorithms (`std::lower_bound`, `std::find_if`, `std::remove_if`) when they express intent, and know the complexity of the container you chose. | Correct by construction, and the complexity is visible. | wk1 |
| CPP-STL-2 | Choose containers by access pattern: `std::deque` or a ring buffer for FIFO time series, `std::vector` by default otherwise. | Data structure choice dominates performance. | wk1 |

### Concurrency

| ID | Rule | Why | Intro |
|---|---|---|---|
| CPP-THR-1 | Every piece of shared mutable state is protected by exactly one documented mutex, or is atomic. Prefer passing messages through queues over sharing state. | Data races are undefined behaviour, not just wrong values. | wk3 |
| CPP-THR-2 | Lock with RAII (`std::scoped_lock`, `std::unique_lock`). Keep critical sections short. Never call callbacks, blocking I/O or unknown code while holding a lock. | Prevents deadlocks and priority inversion. | wk3 |
| CPP-THR-3 | `condition_variable::wait` always takes a predicate. Notify after changing the guarded state. | Spurious wakeups and lost notifications are real. | wk3 |
| CPP-THR-4 | Every thread has one owner that joins it, and shutdown is designed: a stop flag plus a wake-up for blocked waiters. | Detached threads outlive the data they use. | wk3 |
| CPP-THR-5 | Acquire multiple locks through `std::scoped_lock` or in one documented global order. | Deadlock avoidance. | wk3 |
| CPP-THR-6 | Use sequentially consistent atomics unless you can explain why a weaker memory order is correct. | Weak orderings are an expert tool. | wk3 |

## TST: Testing, tooling and measurement

| ID | Rule | Why | Intro |
|---|---|---|---|
| TST-1 | Every behaviour has a test, including the edges: empty, single element, exactly at tolerance, just outside it, out-of-order input and angle wrap-around. | Bugs live at the edges. | wk1 |
| TST-2 | Tests are deterministic: inject time and data, and never `sleep` to synchronise. | A flaky test is worse than no test. | wk1 |
| TST-3 | Run AddressSanitizer and UBSan in the default dev build, and ThreadSanitizer for every concurrency exercise. | They find bugs that tests pass over. | wk1 / wk3 |
| TST-4 | Benchmarks use a release build, warm-up, many iterations, a distribution and a recorded baseline to compare against. | An unmeasured optimisation is a guess. | wk1 (stretch) |
| TST-5 | No dead code: every function is exercised by a test or a caller. | Untested code is unknown code. | wk1 |

## CU: CUDA (introduced week 11)

| ID | Rule | Why |
|---|---|---|
| CU-ERR-1 | Check the return value of every CUDA API call, through one checking macro or function that reports the file, line and error string. | Errors are sticky and asynchronous, so unchecked calls report failures far from their cause. |
| CU-ERR-2 | After every kernel launch, check `cudaGetLastError()`. In debug builds, also synchronise and check for asynchronous errors. | A launch failure is otherwise invisible. |
| CU-LIF-1 | No CUDA calls in global constructors or destructors (before or after `main`). Synchronise all devices before the program terminates. | Device state is undefined at those times (a *mandatory* rule in the safety guidelines). |
| CU-LIF-2 | Wrap device memory, streams and events in RAII types. | Same reason as CPP-OWN-1, on the GPU. |
| CU-KER-1 | Kernel parameters are trivially copyable and passed by value: no references and no virtual types. Kernels bounds-check their index (`if (i < n)`). | Host references are invalid on the device, and the grid is usually larger than the data. |
| CU-PERF-1 | Follow APOD: profile first (`nsys` for the timeline, `ncu` for kernels), estimate the possible gain with Amdahl's law, and optimise the measured hotspot. | The bottleneck is rarely where intuition says it is. |
| CU-PERF-2 | Minimise host↔device transfers. Keep data resident on the device, batch work, and use pinned host memory with asynchronous copies on streams. | PCIe transfers often cost more than the kernel. |
| CU-PERF-3 | Coalesce global memory accesses (neighbouring threads touch neighbouring addresses), and use effective bandwidth (GB/s) as the performance metric. | Memory-bound kernels live or die by their access pattern. |
| CU-PERF-4 | Time GPU work with CUDA events recorded in the stream. If you use CPU timers, synchronise first. Always state whether transfers are included. | Launches are asynchronous, so naive timers measure only the launch. |
| CU-COR-1 | Validate every GPU result against a CPU reference implementation, within a stated tolerance. | Fast and wrong is worthless. |
| CU-LIB-1 | Prefer existing libraries (CUB, Thrust, cuBLAS) over hand-written kernels in production. Write kernels to learn, or when a profile proves the need. | Library kernels are tuned and tested. |

## PY: Python (introduced week 7, Python 3.10+ idioms)

| ID | Rule | Why |
|---|---|---|
| PY-STY-1 | PEP 8 via `ruff format` and `ruff check`, 100 columns. `snake_case` for functions, variables and modules, `PascalCase` for classes, `UPPER_SNAKE` for constants, `_private` for non-public names, no wildcard imports. | Tool-enforced consistency. |
| PY-TYP-1 | Annotate every function, including `-> None`. `mypy --strict` passes. No `Any` and no `# type: ignore`. Use built-in generics (`list[int]`, `X \| None`), `Literal` for fixed value sets and `Protocol` for structural interfaces. | Types are checked documentation and catch API misuse. |
| PY-DAT-1 | Plain records are `@dataclass(frozen=True, slots=True)` where possible. Mutable defaults use `field(default_factory=...)`. | Immutability by default, and no shared-default bug. |
| PY-ERR-1 | Catch the narrowest exception, keep `try` bodies minimal (main logic in `else`), raise built-in exception types, and signal errors with exceptions rather than return codes. Never use `assert` to validate input. | `assert` disappears under `-O`, and broad `except` hides bugs. |
| PY-DOC-1 | Google-style docstrings on public functions, stating units and, for arrays or tensors, shape and dtype (`[batch, height, width, 3] uint8`). | Shape and unit bugs are the most common ML-integration bugs. |
| PY-NAM-1 | Put units in names (`timestamp_ns`, `yaw_rad`). Suffix containers with their location (`_host`, `_device`) when it is ambiguous. | Same reasoning as RT-3. |
| PY-TST-1 | Use `pytest` with `@pytest.mark.parametrize` for edge cases, and keep tests deterministic (TST-2). | Cheap, readable coverage. |
| PY-PERF-1 | Vectorise numeric work (NumPy) instead of Python loops over elements. Measure with `time.perf_counter_ns` or `timeit`. | Python loops over pixels are ~100× slower. |
