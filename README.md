# ml-systems-learning

Learn hands-on C++17, CUDA and Python through small, realistic exercises from ML, sensor, robotics and vehicle-integration problems. It takes one 90-minute session a week. You write the code; Claude Code teaches and reviews.

## Setup (once)

```bash
sudo apt install -y g++ cmake ninja-build clang-format clang-tidy
git switch -c learn/<your-name>        # your work lives on your own branch; main stays the clean template
cmake --preset dev && cmake --build --preset dev
```

## Each week

1. Start Claude Code in the repo with `claude`, and type `/teacher`. It runs a short recall quiz, then gives the session goal.
2. Implement the session in your editor, following `exercises/NN-*/README.md`.
3. Build and test:
   ```bash
   cmake --build --preset dev
   ctest --preset dev -L ex01-s1       # one session's tests; -L ex01 runs the whole exercise
   ./build/dev/exercises/01-sensor-sync/ex01_replay
   ```
4. Write `journal/YYYY-MM-DD.md` with four answers: learned, forgot, difficult, recognise faster next time. Then commit.
5. Type `/reviewer` for a review. The next `/teacher` session starts from it.

## Commands in Claude Code

| Command | What it does |
|---|---|
| `/teacher` | Start a session: recall quiz, then the session goal |
| `/teacher hint` | The next hint, one level more concrete each time |
| `/teacher show me the solution` | Full solution after you confirm (chat only, logged) |
| `/teacher drill` | 5-minute review of "agent-written" code with planted bugs |
| `/reviewer` | Build, test and review your work against `docs/engineering-practices.md` |

Claude cannot edit your exercise code (`exercises/*/src`, `include`, `tests/student_*`); a hook blocks it.

## Author mode

To have Claude write or change exercise material (starter code, tests, teacher notes), launch it with:

```bash
LEARN_AUTHOR_MODE=1 claude
```

This lifts the edit block for that session only.

## Starting an exercise over

```bash
git restore --source main -- exercises/01-sensor-sync/src exercises/01-sensor-sync/include
```

## More

- [docs/how-to-learn-here.md](docs/how-to-learn-here.md): the session format and the rules of the game
- [docs/engineering-practices.md](docs/engineering-practices.md): the coding standard that reviews cite
- Please don't open `exercises/*/.teacher/`; it holds the hints and answers.
