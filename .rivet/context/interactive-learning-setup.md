# Interactive Learning Setup

Status: **approved** by the human on 2026-10-17 (C++17, NVIDIA naming style over the plan's spelling, honour system for `.teacher/`).

Builds on: `12-week-systems-programming-practice-plan.md` (the curriculum) and `docs/engineering-practices.md` (the coding standard, already in place).

## Problem

The 12-week plan says *what* to practise but not how the repository supports it. The student is an experienced systems engineer who integrates ML models into a prototype vehicle with agentic coding. They are rusty at writing code themselves. Their goal is to write and *judge* strong C++, CUDA and Python, so they can steer and review agent-written code better.

They have about **90 minutes a week**. Without structure, an AI assistant in this repo would do what assistants do: write the solution. That would defeat the purpose.

## Desired outcome

The student opens the repo once a week, types one command, and gets a guided, fun 90-minute session. In that session:

- the student writes all the code
- a **teacher** skill keeps them moving with graded hints and never gives the answer unless explicitly asked
- a **reviewer** skill critiques the finished work against one written standard and explains each point twice: in plain terms (for the rusty programmer) and in systems terms (for the experienced engineer)
- progress, recurring weaknesses and reflections accumulate in the repo, so each session starts by retrieving what was learned last time

## Decisions

1. **Two repo-local skills, one shared standard.** The skills live at `.claude/skills/teacher/SKILL.md` and `.claude/skills/reviewer/SKILL.md`. Both ground their judgement in `docs/engineering-practices.md`, which lives outside the context dir so it can evolve. The reviewer cites rule IDs, so feedback is consistent from week to week. Neither skill invents new rules ad hoc. A missing rule is proposed as an addition to the standard.

2. **The student's code is protected from the assistant.** `CLAUDE.md` states the learning contract. A `PreToolUse` hook in `.claude/settings.json` denies `Edit`/`Write`/`NotebookEdit` on the student work paths (`exercises/*/src/**`, `exercises/*/include/**`, `exercises/*/tests/student_*`), with a message pointing to the teacher. Exercise authoring is exempt when Claude is launched with `LEARN_AUTHOR_MODE=1`. Reason: in agentic mode the only reliable "no solution" guarantee is mechanical, not a prompt.

3. **Graded hint ladder, explicit reveal.** The teacher escalates only when asked for more:
   - **L1 nudge**: a question back ("what should happen when both timestamps are equal?")
   - **L2 concept**: which idea or standard tool applies, plus a cppreference/docs pointer
   - **L3 shape**: pseudocode or the structure of the solution, no compilable code for the task
   - **L4 snippet**: one small idiom (≤ 5 lines) that is not the task itself, such as how `std::lower_bound` takes a comparator

   A full solution appears only after the student explicitly asks for it ("show me the solution") and confirms once. It goes in chat and is never written into the student's files. Each reveal is logged in the journal, because a reveal is a learning signal, not a failure.

4. **Teacher notes per exercise, on the honour system.** Each exercise has `.teacher/notes.md` with the intent, common pitfalls, the hint ladder and a reference solution under `.teacher/reference/`. The teacher reads these to give hints that are on point. The student agrees not to open `.teacher/`, and that is enough for a self-directed adult.

5. **Sessions are sized to one 90-minute block, not to calendar weeks.** The plan's 10 / 50 / 20 / 10 format is kept: recall / implement / debug-test-measure / reflect. Each exercise is cut into **sessions** of about 50 minutes of coding (roughly 30–80 lines). If the student falls behind, the next session simply continues: consistency over completion, as the plan says.

6. **Review closes the loop into the next session.** Each session ends with a commit and a reflection. The reviewer then runs and writes `exercises/NN-*/reviews/session-K.md`. The *next* session's 10-minute recall is built from that review plus the concept list: the teacher quizzes without notes, and the student may fix one must-fix finding.

7. **Review drill for agentic-coding skill (optional, about 5 minutes).** Once an exercise session is done, the teacher can produce a plausible "agent-written" version of the same function with 2–3 seeded flaws (dangling reference, unsigned underflow, unbounded queue). The student hunts for them. This trains the motivating skill directly: reviewing AI code.

8. **Language and toolchain.**
   - C++17, the language level of MISRA C++:2023 and of most vehicle and inference C++ code the student will read
   - CMake + Ninja with `CMakePresets.json` presets `dev` (ASan + UBSan, `-Werror` warning set from CPP-ERR-3), `tsan` and `release`
   - GoogleTest via `FetchContent`
   - `clang-format` / `clang-tidy` configs matching the reference
   - Python ≥ 3.10 idioms with `pyproject.toml`, pytest, ruff and mypy, used from week 7

   Naming follows CPP-STY-2, so the plan's `timestamp_ns` field is spelled `timestampNs` in this repo.

9. **GPU work happens on another machine.** This machine has no GPU or `nvcc`. The build is CPU-only by default, and the CUDA targets of weeks 11–12 are added only when CMake finds a CUDA compiler. The student clones the repo on a GPU machine, runs the CUDA exercise there, and commits code, benchmark output and exported profiler summaries as text. Review and teaching work from those committed artefacts, so the skills never need GPU access.

10. **Fun comes from realism and visible results.** Every exercise opens with a short vehicle scenario, for example: "Perception says boxes lag in turns. Why?" Each exercise also has a tiny replay/simulation driver that prints a live stats summary (matches, drops, latency p50/p99), so progress is visible and measurable rather than a green test count alone.

## Repository shape

```
CLAUDE.md                      learning contract + pointers to skills and standard
CMakeLists.txt, CMakePresets.json, .clang-format, .clang-tidy, pyproject.toml
.claude/settings.json          hook protecting student paths
.claude/skills/teacher/SKILL.md
.claude/skills/reviewer/SKILL.md
docs/engineering-practices.md  the standard (rule IDs)
docs/how-to-learn-here.md      one-page student guide: session flow, commands, hint ladder
exercises/NN-slug/
  README.md                    scenario, sessions, concepts to recall, acceptance per session
  include/ src/                starter code (student edits)
  tests/                       provided tests, labelled per session; tests/student_*.cpp are the student's own
  app/                         replay/simulation driver that prints stats
  .teacher/notes.md, .teacher/reference/
  reviews/session-K.md         reviewer output
journal/
  progress.md                  concept mastery + recurring findings (teacher/reviewer maintain)
  YYYY-MM-DD.md                student-written reflection (four questions from the plan)
```

## Teacher skill: behaviour contract

- **Invocation.** `/teacher` starts or resumes a session. `/teacher hint` asks for the next rung on the ladder. `/teacher show me the solution` starts a reveal. `/teacher drill` starts the review drill.
- **Start.** Read `journal/progress.md`, the latest review and the exercise README, then run a 10-minute recall: 3–5 questions answered without notes, with at least one about the last review's findings. After that, state the session goal and the acceptance tests in two lines.
- **During the session.** Answer at the lowest helpful rung. Never edit student paths. Run builds and tests when asked and explain compiler errors, saying what the message means and where to look, not how to fix the code. Mention the "stuck for about 20 minutes" rule from the plan.
- **Calibration.** Treat the student as a peer in systems thinking. Do not explain latency, clocks or pipelines from scratch; map language mechanics onto those (for example, "a dangling reference is a use-after-free of a buffer you thought you owned"). Explain C++ and Python mechanics patiently and concretely.
- **End.** Prompt the four reflection questions. The student writes `journal/YYYY-MM-DD.md` themselves, because retrieval is the point. Then the teacher reminds them to commit and offers `/reviewer`.

## Reviewer skill: behaviour contract

- **Evidence before opinion.** Build with the `dev` preset (and `tsan` from week 3), run all tests and the replay driver, and record the results at the top of the review. Do not review code that does not compile; instead, explain the first compiler error.
- **Scope.** Review the student's diff for the session. Apply only rules introduced so far (see the "Intro" column). Later rules may appear as at most one "preview" line.
- **Format.** At most 5 findings, ranked by severity (blocker → nit), plus at least one *praise*. Each finding:
  ```
  [severity] RULE-ID  file:line  — one-line title
  In plain terms: what is wrong, as you would explain it to someone learning to code.
  Systems view:   what this would do in a vehicle/real-time pipeline (latency, data loss, safety).
  How to see it:  a test, input or tool (sanitizer, benchmark) that exposes it.
  Your move:      a question or direction — not rewritten code.
  ```
  Rewritten code is shown only if the student explicitly asks.
- **Persistence.** Write `reviews/session-K.md` and update the recurring-findings section of `journal/progress.md` (rule ID, count, last seen). Recurring IDs are what the teacher quizzes.

## First exercise: `01-sensor-sync` (plan weeks 1–2)

**Scenario.** A 10 Hz camera and 50 Hz odometry arrive with nanosecond timestamps, jitter, occasional drops and out-of-order messages. Perception needs the vehicle pose *at the moment each frame was captured*.

**Session 1: Nearest match.** The student implements an `OdometryBuffer`:
- `add()`, which rejects out-of-order samples and counts them
- `findNearest(timestampNs, tolerance)`, which returns `std::optional<Odometry>`
- `discardBefore(timestampNs)`, which keeps memory bounded

Concepts: structs, `const&`, `std::deque`, `std::optional`, `std::chrono` durations, linear search, unsigned-difference pitfalls (CPP-TYP-2). The replay driver prints the match rate.

**Session 2: Interpolate and account.** The student adds:
- `interpolate(timestampNs)` between the bracketing samples, including **yaw wrap-around at ±π** (the "boxes lag in turns" bug)
- a capacity with a drop policy (RT-2)
- latency, jitter and drop statistics, with p50/p99 (RT-4, RT-7)

**Stretch (optional extra session).** Replace the linear search with `std::lower_bound` and benchmark both with the `release` preset (TST-4).

**Provided tests** are labelled `ex01-s1` and `ex01-s2` and cover the edge cases from TST-1. The student adds at least one `tests/student_*.cpp` test per session.

## Boundaries and invariants

- The assistant never writes or edits the student work paths outside author mode. This is the core invariant of the repo.
- The skills, `CLAUDE.md` and `docs/` explain rules by their reasons, not by authority. The student must be able to argue each rule.
- The curriculum order and content stay as in the 12-week plan. This concept defines the mechanics only. Later exercises (02–06) follow the same layout, authored one at a time just before they are needed, not all up front.
- Nothing in the repo states the student's career goals or employers.

## Acceptance

- `cmake --preset dev && cmake --build --preset dev` succeeds on this machine (g++ 15, no GPU) with the starter code. `ctest --preset dev -L ex01-s1` runs, and the tests fail only because of unimplemented stubs.
- Building against the teacher reference (CMake option `-DLEARN_USE_REFERENCE=ON`) passes all `ex01-s1` and `ex01-s2` tests under the `dev` (ASan/UBSan) preset. This proves the tests are valid and solvable.
- In a normal Claude session, an `Edit` of `exercises/01-sensor-sync/src/*` is denied by the hook with a message that points to `/teacher`. With `LEARN_AUTHOR_MODE=1` it is allowed.
- `/teacher` in a fresh session runs the recall step, states the session-1 goal and answers a "how do I find the closest timestamp?" request at rung L1 or L2, without compilable solution code.
- `/reviewer` on a deliberately flawed sample submission (included as a skill eval fixture) reports the seeded unsigned-underflow and missing-`const` issues with their rule IDs in the finding format above, and writes a review file.
- `docs/how-to-learn-here.md` exists, and the skills reference `docs/engineering-practices.md`. `clang-format --dry-run -Werror` passes on the provided non-student code.

## Assumptions and open points

- **Tools to install before implementation:** `cmake`, `ninja-build`, `clang-format` and `clang-tidy` (apt). GoogleTest is fetched at configure time, which needs network once. Not blocking: the implementer can install the tools or ask.
- **GPU machine details** (OS, GPU, CUDA version) are needed only by exercise 06. They are not blocking now.
- **Python environment:** a standard `venv`, since `uv` is not installed. This is decided when exercise 04 is authored.
