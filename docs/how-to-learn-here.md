# How to learn here

One protected 90-minute block a week. You write all the code; Claude teaches and reviews.

## The deal

- **You write the code.** Claude cannot edit your exercise code (`exercises/*/src`, `exercises/*/include`, `exercises/*/tests/student_*`); a hook blocks it.
- **Hints, not answers.** Ask `/teacher hint` when stuck. If you have been stuck for about 20 minutes, ask for a hint rather than grinding.
- **Answers only on request.** `/teacher show me the solution` reveals it after one confirmation, in chat. Reveals are logged; they are data, not failure.
- **Honour system.** Don't open `exercises/*/.teacher/`, which holds hints and reference solutions.
- **One standard.** Everything is judged against [engineering-practices.md](engineering-practices.md). Every review comment cites a rule ID from it, and you should be able to argue every rule.

## A session

| Minutes | What | Command |
|---|---|---|
| 10 | **Recall.** Answer questions without notes, including the findings from your last review. Optionally fix one must-fix finding. | `/teacher` |
| 50 | **Implement** the session's tasks from the exercise README. | your editor; `/teacher hint` when stuck |
| 20 | **Debug, test, measure.** Get the session's tests green, add one test of your own, run the replay. | `cmake --build --preset dev`, `ctest --preset dev -L ex01-s1` |
| 10 | **Reflect.** Write `journal/YYYY-MM-DD.md` with four answers: what I learned, what I forgot, what felt difficult, what I'd recognise faster next time. Then commit. | `git commit` |

After committing, run `/reviewer`. Read the review whenever you like; the next `/teacher` session starts from it.

Optionally, `/teacher drill` gives you about 5 minutes of reviewing "agent-written" code with planted bugs. That is the skill you are really here for.

Fell behind? Just continue next week from where you stopped. Consistency beats completion.

## Builds

```bash
cmake --preset dev && cmake --build --preset dev    # debug + AddressSanitizer + UBSan; warnings are errors
ctest --preset dev -L ex01-s1                       # one session's tests (-L ex01 for the whole exercise)
cmake --preset tsan && cmake --build --preset tsan  # ThreadSanitizer, from exercise 02
cmake --preset release && cmake --build --preset release   # for benchmarks
cmake --preset tidy && cmake --build --preset tidy  # clang-tidy diagnostics
clang-format -i <file>                              # formatting is never a review topic
```

## GPU exercises

This machine has no GPU. For the CUDA exercise (weeks 11–12), clone the repo on a machine with a GPU and CUDA toolkit. There, run the exercise and commit the code, benchmark output and profiler summaries (exported as text). Teaching and review then work from those committed files on any machine.

## Layout

```
docs/engineering-practices.md   the standard (rule IDs)
exercises/NN-name/README.md     scenario, sessions, done-when
exercises/NN-name/reviews/      reviews of your work
journal/progress.md             where you are, recurring findings, reveals
journal/YYYY-MM-DD.md           your reflections
```
