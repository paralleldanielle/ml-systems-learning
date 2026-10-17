# Expected review: flawed-session-1

Seeded flaws in `odometryBuffer.cpp`, for a session-1 review of `01-sensor-sync`:

| Must report | Rule | Severity | Where |
|---|---|---|---|
| Yes | CPP-TYP-2: `sample.timestampNs - timestampNs` wraps around when the query is newer than the sample | blocker | `findNearest`, distance line |
| Yes | CPP-CON-1: `distance` is never modified but not `const` | minor | `findNearest`, distance line |
| Should | CPP-CON-2: `for (auto sample : mSamples)` copies every sample | minor | `findNearest`, loop |
| Should | CPP-STY-3: magic `18446744073709551615ULL` (use the optional's emptiness, or `std::numeric_limits`) | minor | `findNearest` |
| Praise | RT-4 / CPP-API-1: rejected samples are counted, and absence is `std::nullopt` | praise | `add`, `findNearest` |

Evidence the review should cite: `ctest --preset dev -L ex01-s1` fails `QueryAfterNewestSample`, `PicksCloserOfTwoSamples` and `ToleranceIsInclusive`, among others.

It must **not** count session-2 items (interpolation, capacity eviction) as findings.

## Running the eval

```bash
git worktree add ../reviewer-eval HEAD
cp .claude/skills/reviewer/evals/flawed-session-1/odometryBuffer.cpp ../reviewer-eval/exercises/01-sensor-sync/src/
cd ../reviewer-eval && claude      # then: /reviewer 01-sensor-sync 1
# afterwards: git worktree remove --force ../reviewer-eval
```

Pass: both "Must report" rows appear with their rule IDs in the finding format, and the review file is written.
