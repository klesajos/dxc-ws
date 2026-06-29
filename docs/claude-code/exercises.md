> 🌍 Read this in: **English** | [Česky](exercises.cs.md)

# Exercise catalog: one feature per mechanism

You've worked through all six extension mechanisms ([Skills](01-skills.md),
[Hooks](02-hooks.md), [MCP](03-mcp.md), [Plugins](04-plugins.md),
[Subagents](05-agents.md), [Workflows](06-workflows.md)). This catalog turns
them into practice: **each game feature below is a vehicle for exercising one
specific mechanism.** The point isn't only the feature — it's getting the
mechanism into your hands.

Each exercise is **self-contained**: it gives you a paste-ready **starter
prompt**, the exact **files** you'll touch, the bug (where there is one)
explained inline, a **step-by-step walkthrough with the full solution code**, and
a **done check**. The linked guide is optional depth.

## How to use this catalog

1. **Pick a row** from the at-a-glance table below.
2. **Open the linked guide** for that mechanism if you want the *how* and *why* —
   otherwise skip it; the exercise stands on its own.
3. **`Shift+Tab`** to switch Claude Code into plan or accept-edits mode (so you
   review or auto-apply edits the way you prefer).
4. **Paste the Starter prompt** from the exercise verbatim.
5. **Stuck, or want to compare?** Each exercise ends with a **Walkthrough** — the
   full solution, step by step, with copy-pasteable code.
6. **Run the Done-check command** to confirm it worked.

Every exercise states its **goal**, the **mechanism** it teaches, the **files**
you'll touch, a paste-ready **starter prompt**, and a **done check**. The done
check always includes this baseline — the project still builds and the suite is
green:

```bash
cmake -S . -B build && cmake --build build -j && ctest --test-dir build --output-on-failure
```

**Easiest on-ramp:** this repo ships with two real, deliberate bugs (both used in
Exercise 5). Fixing them needs almost no new code and is the fastest way to feel
a mechanism work:

- **`isGameOver()`** (`src/board.cpp`, lines 122–133) only checks for an empty
  cell, so a full board that still has a mergeable pair is wrongly reported as
  game over.
- **The `changed` flag** in `src/game.cpp` (line 31) is computed from
  `board_.move(...)` and then ignored, so `spawnRandom()` runs unconditionally
  (line 34) — a tile spawns even after a move that changed nothing.

Difficulty: 🟢 Easy · 🟡 Medium · 🔴 Hard.

## At a glance

| Mechanism | Feature exercise | Difficulty | Touches |
|---|---|---|---|
| **Skill** | A `renderer-style` skill, then colour tiles + a move counter | 🟢–🟡 | `renderer.{hpp,cpp}`, `game.{hpp,cpp}` |
| **Hook** | A `ctest` Stop hook that blocks on red, then "configurable win value" behind it | 🟡 | `.claude/settings.json`, `board.{hpp,cpp}` |
| **MCP** | High-score persistence via the `memory` MCP server | 🟡 | `game.cpp`, `renderer.{hpp,cpp}`, `.mcp.json` |
| **Plugin** | Package the undo workflow as a bundled `/2048-dev:undo` **skill** | 🟡 | `plugins/2048-dev/` |
| **Subagent** | Undo feature: planned, tested, reviewed by the agents; fix the two bugs | 🟢–🟡 | `board`, `game`, `input` |
| **Workflow** | AI auto-solver: parallel heuristics, benchmarked, judge-picked | 🔴 | new `ai.{hpp,cpp}`, `board` |

---

## 1. Skill 🟢–🟡 — teach the renderer's conventions, then use them

**Goal.** Write a new skill `renderer-style` that captures this project's ANSI /
terminal-drawing conventions (the escape codes in `renderer.cpp`, the
`kCellWidth` alignment, the score header). Then, *with the skill active*, add
**colour tiles** (different ANSI colour per tile value) and a **move counter**
shown in the header.

**Mechanism it teaches.** Authoring a [skill](01-skills.md) — a description that
triggers on the right requests, a body that briefs Claude on conventions — and
then watching it load on demand. Contrast with `board-tests`: same shape, new
domain.

**Files.** `.claude/skills/renderer-style/SKILL.md` (new), `src/renderer.{hpp,cpp}`,
`src/game.{hpp,cpp}` (the move counter lives on the `Game` class, **not** on
`Board` — `Board` stays I/O-free).

**Inline context.** `src/renderer.cpp` clears the screen with `\x1b[2J\x1b[H`
(line 16), prints the `  2048  —  score:` header (line 17), and lays out the grid
as `kCellWidth`-wide right-aligned columns via `std::printf("%*d", kCellWidth, value)`
(lines 11, 25). `Renderer::draw` currently takes only `const Board&`
(`src/renderer.hpp:13`) — to show a move counter you pass the count in as a new
argument, since `Board` has no move count and must not gain I/O.

**Starter prompt.**
> *"Create a skill at `.claude/skills/renderer-style/SKILL.md` that documents the
> terminal-drawing conventions in `src/renderer.cpp`: the `\x1b[2J\x1b[H`
> clear-and-home escape sequence, the `  2048  —  score:` header, and the
> `kCellWidth`-wide right-aligned columns printed with
> `std::printf("%*d", kCellWidth, value)`. Write the `description` as a trigger
> for any renderer/ANSI-output work in this repo."*

Then, once the skill is active:
> *"With the renderer-style skill, add a per-value ANSI colour to each tile and a
> `moves: N` counter in the header. Keep `Board` I/O-free: store the move count on
> the `Game` class in `src/game.{hpp,cpp}` and pass it into `Renderer::draw` in
> `src/renderer.{hpp,cpp}` alongside the board."*

**Walkthrough (full solution).**

1. **Write the skill** at `.claude/skills/renderer-style/SKILL.md`:

```markdown
---
name: renderer-style
description: Terminal-drawing conventions for this 2048 repo — ANSI escape codes, the score header, and kCellWidth column alignment. Use for any work on src/renderer.cpp or other ANSI/terminal output.
---

# Renderer style

This project draws the board to stdout with raw ANSI escape codes. Match these
conventions in any renderer work:

- **Clear + home:** begin a frame with `\x1b[2J\x1b[H`.
- **Header:** one line — `  2048  —  score: <n>` — then a blank line.
- **Grid:** each tile is a `kCellWidth`-wide, right-aligned column printed with
  `std::printf("%*d", kCellWidth, value)`; empty cells print `" ."`.
- **Footer:** the hint `  Arrows / WASD to move,  q to quit`.
- **Colour:** wrap a value in an SGR colour with `\x1b[<code>m … \x1b[0m`; always
  reset with `\x1b[0m` so colour never leaks past the cell.
- Keep all of this in `Renderer`; `Board` stays I/O-free, so the move count is
  passed into `Renderer::draw`.
```

2. **Widen the `draw` signature** so the renderer receives the move count
   (`src/renderer.hpp`):

```cpp
    // Clears the screen and renders the board, score and move counter.
    void draw(const Board& board, int moves) const;
```

3. **Add a colour table and the counter** in `src/renderer.cpp` — `colorFor` in
   the anonymous namespace, `moves` in the header, each tile wrapped in colour:

```cpp
namespace {
constexpr int kCellWidth = 6;  // characters per tile, e.g. "  2048"

// ANSI SGR colour for each tile value; bigger tiles get hotter colours.
const char* colorFor(int value) {
    switch (value) {
        case 2:
            return "\x1b[37m";  // white
        case 4:
            return "\x1b[36m";  // cyan
        case 8:
            return "\x1b[32m";  // green
        case 16:
            return "\x1b[33m";  // yellow
        case 32:
            return "\x1b[35m";  // magenta
        case 64:
            return "\x1b[31m";  // red
        case 128:
        case 256:
        case 512:
            return "\x1b[94m";  // bright blue
        default:
            return "\x1b[91m";  // bright red (1024+)
    }
}
}  // namespace

void Renderer::draw(const Board& board, int moves) const {
    // Clear screen and move the cursor to the top-left corner.
    std::cout << "\x1b[2J\x1b[H";
    std::cout << "  2048  —  score: " << board.score() << "   moves: " << moves
              << "\n\n";

    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            const int value = board.at(r, c);
            if (value == 0) {
                std::printf("%*s", kCellWidth, " .");
            } else {
                // Colour the tile, then reset so colour never leaks past it.
                std::printf("%s%*d\x1b[0m", colorFor(value), kCellWidth, value);
            }
        }
        std::cout << "\n\n";
    }

    std::cout << "  Arrows / WASD to move,  q to quit\n";
    std::cout.flush();
}
```

4. **Store the count on `Game`, not `Board`** — add to the private members in
   `src/game.hpp`:

```cpp
    int moves_ = 0;
```

5. **Count real moves and pass them in** (`src/game.cpp`) — increment only when
   the board changed, then redraw with the count:

```cpp
void Game::run() {
    renderer_.draw(board_, moves_);

    while (true) {
        const Command cmd = input_.next();
        if (cmd == Command::Quit) {
            break;
        }
        if (cmd == Command::None) {
            continue;
        }

        const bool changed = board_.move(toDirection(cmd));
        if (changed) {
            ++moves_;
            board_.spawnRandom();
        }
        renderer_.draw(board_, moves_);
```

   Guarding the spawn with `if (changed)` also fixes the no-op spawn that
   Exercise 5 names as a bug.

**Done check.** Build + `ctest` green; the game renders coloured tiles and a
`moves: N` counter. (Keep rendering out of `Board` — colours and the move count
flow through `Renderer`/`Game`.)

## 2. Hook 🟡 — make tests un-skippable, then add a gated feature

**Goal.** Add a hook that runs `ctest` and **blocks on red** — a `Stop` hook so a
session can't end with failing tests. Then build a **configurable win value**
(e.g. play to 1024 or 4096 instead of 2048) and let the hook guarantee you never
finish with a broken suite.

**Mechanism it teaches.** A [hook](02-hooks.md) that enforces something *every
time*, with no model discretion — the complement to the existing format hook.

**Files.** `.claude/hooks/gate-tests.sh` (new), `.claude/settings.json` (register
it under `hooks.Stop`), `src/board.{hpp,cpp}` (`kWinValue` becomes configurable).

**Inline context.** `.claude/hooks/run-tests.sh` already runs `ctest` on `Stop`,
but it's *advisory*: it prints a pass/fail line and always `exit 0`. A blocking
hook instead **exits non-zero** (or prints `{"decision": "block", "reason": "..."}`
on stdout) when tests fail. The win value is hard-coded as
`static constexpr int kWinValue = 2048;` at `src/board.hpp:41`.

**Starter prompt.**
> *"Create `.claude/hooks/gate-tests.sh` and `chmod +x` it. Model it on the
> existing `.claude/hooks/run-tests.sh`, but instead of always `exit 0`, make it
> **block**: when `ctest` reports any failing test, exit non-zero (or print
> `{"decision": "block", "reason": "tests are red"}` on stdout) so a `Stop` can't
> end the session on red. Register it under `hooks.Stop` in
> `.claude/settings.json` alongside the existing `run-tests.sh` entry."*

Then, with the gate in place:
> *"Make the win value configurable. `kWinValue` is `static constexpr int
> kWinValue = 2048;` at `src/board.hpp:41` — let the game be played to 1024 or
> 4096 instead, threaded through `Board` without adding any I/O to it."*

**Walkthrough (full solution).**

1. **Write the blocking hook** at `.claude/hooks/gate-tests.sh`, then
   `chmod +x .claude/hooks/gate-tests.sh`:

```bash
#!/usr/bin/env bash
# Stop hook: run the suite and BLOCK the Stop when it is red, so a session can't
# end on failing tests. Models run-tests.sh, but emits a block decision instead
# of always exiting 0.
set -euo pipefail

cat >/dev/null  # drain the Stop-event JSON on stdin

project_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}"
build_dir="$project_dir/build"

# Nothing built yet — let the Stop through.
if [ ! -d "$build_dir" ] || ! command -v ctest >/dev/null 2>&1; then
    exit 0
fi

if ctest --test-dir "$build_dir" >/dev/null 2>&1; then
    exit 0
fi

# Tests are red: block the Stop and tell Claude how to proceed.
echo '{"decision": "block", "reason": "Tests are failing — run ctest --test-dir build --output-on-failure and fix them before stopping."}'
exit 0
```

2. **Register it** under `hooks.Stop` in `.claude/settings.json`, alongside the
   existing advisory `run-tests.sh`:

```json
    "Stop": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/run-tests.sh"
          },
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/gate-tests.sh"
          }
        ]
      }
    ]
```

3. **Make the win value a per-game field** (`src/board.hpp`) — replace the
   hard-coded `kWinValue` constant with a default plus a member, and give both
   constructors a `winValue` parameter:

```cpp
    // Starts an empty board and seeds it with two random tiles. The target
    // tile defaults to 2048 but can be set (e.g. play to 1024 or 4096).
    explicit Board(int winValue = kDefaultWinValue);

    // Builds a board from an explicit grid (handy for tests). No tiles are
    // spawned, so the state is fully deterministic.
    explicit Board(Grid grid, int score = 0, int winValue = kDefaultWinValue);
```

```cpp
    bool hasWon() const;  // true once any tile reaches winValue()

    int score() const { return score_; }
    const Grid& grid() const { return grid_; }
    int at(int row, int col) const { return grid_[row][col]; }
    int winValue() const { return winValue_; }

    // Default target tile; the active value lives in winValue_ so it can be
    // changed per game without adding any I/O to Board.
    static constexpr int kDefaultWinValue = 2048;

private:
    Grid grid_{};
    int score_ = 0;
    int winValue_ = kDefaultWinValue;
    std::mt19937 rng_;
```

4. **Thread it through the constructors and `hasWon`** (`src/board.cpp`):

```cpp
Board::Board(int winValue) : winValue_(winValue), rng_(std::random_device{}()) {
    spawnRandom();
    spawnRandom();
}

Board::Board(Grid grid, int score, int winValue)
    : grid_(grid), score_(score), winValue_(winValue), rng_(std::random_device{}()) {}
```

```cpp
            if (value >= winValue_) {
                return true;
            }
```

5. **Let `Game` choose the value** (`src/game.hpp` then `src/game.cpp`):

```cpp
    explicit Game(int winValue = Board::kDefaultWinValue);
```

```cpp
Game::Game(int winValue) : board_(winValue) {}
```

   …and report the real target when the player wins:

```cpp
            renderer_.message("You reached " + std::to_string(board_.winValue()) +
                              "! Keep going or press q.");
```

6. **Pass it in from the command line** (`src/main.cpp`):

```cpp
#include <cstdlib>

#include "game.hpp"

int main(int argc, char** argv) {
    // Optional first argument overrides the win value, e.g. ./2048 1024
    int winValue = g2048::Board::kDefaultWinValue;
    if (argc > 1) {
        winValue = std::atoi(argv[1]);
    }

    g2048::Game game(winValue);
    game.run();
    return 0;
}
```

7. **Add a regression test** (`tests/test_board.cpp`):

```cpp
TEST_CASE("hasWon respects a custom win value") {
    Board board(Grid{{{1024, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
                0, 1024);
    REQUIRE(board.hasWon());
}
```

   Now `./build/2048 1024` wins at 1024, and with red tests the `Stop` hook
   refuses to let the session end.

**Done check.** Deliberately break a test → the `Stop` hook blocks. Fix it → the
session ends cleanly. Build + `ctest` green with the new win value.

## 3. MCP 🟡 — persist the high score across sessions

**Goal.** Use the already-configured **`memory`** MCP server (see `.mcp.json`) to
store and retrieve the best score, so it survives across runs. Claude reads/writes
the high score through the MCP server; the game displays `best: N`.

**Mechanism it teaches.** Giving Claude a [capability](03-mcp.md) it doesn't have
natively (durable memory) via an MCP server — and wiring its data into the app.

**Files.** `src/game.cpp` (read/write the best score in `Game::run`),
`src/renderer.{hpp,cpp}` (the header that shows `best: N`), `.mcp.json` (already
has `memory`; confirm it's connected with `/mcp`).

**Inline context.** `.mcp.json` already registers a `memory` stdio server
(`@modelcontextprotocol/server-memory`). The score header is drawn in
`src/renderer.cpp` (line 17) and `Renderer::draw` takes only `const Board&`
(`src/renderer.hpp:13`) — so the best score has to be passed into `Renderer::draw`
to appear in the header.

**Starter prompt.**
> *"Run `/mcp` and confirm the `memory` server is connected (it's already in
> `.mcp.json`). Then use that server to persist a best score across sessions: in
> `Game::run` (`src/game.cpp`) read the stored best at start and write it back
> whenever the current score beats it. Surface it as `best: N` in the header —
> the header lives in `src/renderer.cpp`, so pass the best score into
> `Renderer::draw` (`src/renderer.{hpp,cpp}`) alongside the board."*

**Walkthrough (full solution).**

Persistence lives in the `memory` MCP server (Claude's durable store); the C++
side only *shows* the best and *reports* the final score so Claude can save it.
The two meet at an environment variable, `BEST_SCORE`.

1. **Confirm the server.** Run `/mcp` and check `memory` is connected (already in
   `.mcp.json`).

2. **Show `best: N` in the header** (`src/renderer.hpp` then `src/renderer.cpp`):

```cpp
    // Clears the screen and renders the board, score and best score.
    void draw(const Board& board, int best) const;
```

```cpp
    std::cout << "  2048  —  score: " << board.score() << "   best: " << best
              << "\n\n";
```

3. **Seed the best from the environment** (`src/game.hpp` — add `int best_ = 0;`
   to the private members and a constructor; then `src/game.cpp`):

```cpp
    Game();
```

```cpp
#include "game.hpp"

#include <algorithm>
#include <cstdlib>

namespace g2048 {

Game::Game() {
    if (const char* env = std::getenv("BEST_SCORE")) {
        best_ = std::atoi(env);
    }
}
```

4. **Track the running best and report it at the end** (`src/game.cpp`,
   `Game::run`):

```cpp
void Game::run() {
    renderer_.draw(board_, std::max(best_, board_.score()));

    while (true) {
        const Command cmd = input_.next();
        if (cmd == Command::Quit) {
            break;
        }
        if (cmd == Command::None) {
            continue;
        }

        board_.move(toDirection(cmd));
        board_.spawnRandom();

        const int best = std::max(best_, board_.score());
        renderer_.draw(board_, best);

        if (board_.hasWon()) {
            renderer_.message("You reached 2048! Keep going or press q.");
        }
        if (board_.isGameOver()) {
            const int finalBest = std::max(best_, board_.score());
            renderer_.message("Game over. Final score: " +
                              std::to_string(board_.score()) +
                              "  (best: " + std::to_string(finalBest) + ")");
            break;
        }
    }
}
```

5. **Wire MCP to the env var** — this is the workshop's point, with Claude as the
   bridge:
   - *First run:* `BEST_SCORE=0 ./build/2048`. When it prints the final best, ask
     Claude *"store my 2048 best score in the memory server"* — Claude calls the
     `memory` server (an entity `2048-best-score` with the value as an
     observation).
   - *Next run:* ask *"what's my 2048 best score?"* — Claude reads it back from
     `memory` — then launch `BEST_SCORE=<that value> ./build/2048` and the header
     shows `best: N` carried across sessions.

**Done check.** Play a game, reach a score, start a new session → the best score
persists and shows as `best: N`. Build + `ctest` green.

## 4. Plugin 🟡 — package the undo workflow as a bundled skill

**Goal.** Extend the `2048-dev` plugin with a new **bundled skill** —
`plugins/2048-dev/skills/undo/SKILL.md` — that drives an undo workflow (keep a
history of board states, revert the last move), shipped alongside the existing
`build-test` skill so the whole team shares the same `/2048-dev:undo`.

**Mechanism it teaches.** Growing a [plugin](04-plugins.md) — adding a bundled
skill under `plugins/2048-dev/skills/`, a version bump, validation — so a whole
team shares the same workflow. **Skills are the modern path**: custom commands
have merged into Skills (the [cheat-sheet](cheatsheet.md) explains the
command→skill convergence), so build new plugin pieces as skills, not legacy
`commands/` files.

**Files.** `plugins/2048-dev/skills/undo/SKILL.md` (new),
`plugins/2048-dev/.claude-plugin/plugin.json` (bump `version` 1.2.0 → 1.3.0).
Validate with `claude plugin validate .`.

**Inline context.** The plugin already bundles one skill at
`plugins/2048-dev/skills/build-test/SKILL.md` and is at `version` `1.2.0` in
`plugins/2048-dev/.claude-plugin/plugin.json`. A skill folder
`skills/undo/` becomes the command `/2048-dev:undo` (the prefix is the plugin
name). There is intentionally no `commands/` directory — use a skill.

**Starter prompt.**
> *"Extend the `2048-dev` plugin with a new bundled skill at
> `plugins/2048-dev/skills/undo/SKILL.md`, modelled on the existing
> `plugins/2048-dev/skills/build-test/SKILL.md`. It should drive the undo feature:
> keep a board-state history and revert the last move. Bump `version` in
> `plugins/2048-dev/.claude-plugin/plugin.json` from `1.2.0` to `1.3.0`, then run
> `claude plugin validate .`. Use a Skill, not a legacy `commands/` file."*

**Walkthrough (full solution).**

1. **Add the bundled skill** at `plugins/2048-dev/skills/undo/SKILL.md`, modelled
   on the existing `build-test` skill. (The undo *code* is Exercise 5 — this skill
   packages the workflow so the whole team shares `/2048-dev:undo`.)

```markdown
---
name: undo
description: Add or drive the 2048 undo feature — keep a history of prior board states and revert the last move. Use when working on undo/history in this repo.
disable-model-invocation: true
---

# Undo

Drive the 2048 undo feature: keep a stack of prior board states and revert the
last move on demand.

1. **History.** Before each board-changing move, push a snapshot (grid + score)
   onto a history stack in `Board`. Cap the depth so it can't grow unbounded.
2. **Revert.** `Board::undo()` pops the last snapshot and restores grid + score;
   it returns false when the history is empty.
3. **Key.** Add a `Command::Undo` value in `src/input.hpp`, map `u` to it in
   `src/input.cpp`, and handle it in `Game::run` — undo, redraw, do **not** spawn
   a tile.
4. **Verify.** `cmake --build build -j && ctest --test-dir build --output-on-failure`.
   Add a test: a move then an undo restores the exact prior grid and score.

Report what changed and the test result.
```

2. **Bump the plugin version** in
   `plugins/2048-dev/.claude-plugin/plugin.json` (`1.2.0` → `1.3.0`):

```json
  "version": "1.3.0",
```

3. **Validate the plugin** so the new skill is well-formed and discoverable:

```bash
claude plugin validate .
```

**Done check.** `claude plugin validate .` passes; `/2048-dev:undo` autocompletes
and loads. Build + `ctest` green.

## 5. Subagent 🟢–🟡 — delegate an undo feature across the three agents

**Goal.** Build a real **undo** feature (keep a history of board states; `u`
reverts the last move) by *delegating each part to the right agent from
[Example 5](05-agents.md)*:

- `game-explorer` traces where moves are applied and where history would live.
- `board-test-writer` writes the tests for the undo behaviour first.
- `cpp-reviewer` reviews your diff before you commit.

As a warm-up, fix the **two known bugs** (`isGameOver()` and the `changed` flag)
using `cpp-reviewer` to confirm the fix and systematic debugging to reason it
through.

**Mechanism it teaches.** Using [subagents](05-agents.md) as a team: a read-only
cartographer, a write-capable tester, and a read-only reviewer — each in its own
isolated context, each with the right tools.

**Files.** `src/board.{hpp,cpp}` (history + revert), `src/game.cpp` (handle the
undo command), `src/input.{hpp,cpp}` (`src/input.hpp` needs a new `Command` value
for undo; `src/input.cpp` maps a key to it).

**Inline context (the two bugs).**
- **`isGameOver()`** at `src/board.cpp:122–133` returns `true` as soon as it finds
  no empty cell — it never checks for mergeable neighbours, so a full-but-playable
  board is wrongly "game over". The matching test is the scaffold TODO at
  `tests/test_board.cpp:105`.
- **The `changed` flag**: `Game::run` computes
  `const bool changed = board_.move(toDirection(cmd));` (`src/game.cpp:31`) but
  then calls `board_.spawnRandom();` unconditionally (`src/game.cpp:34`). So a
  no-op move still spawns a tile, and `changed` is an unused variable. This bug
  lives in `Game::run()`, which has **no unit-test harness**.

**Starter prompt — bug A (isGameOver).**
> *"Hand the scaffold TODO at `tests/test_board.cpp:105` to `board-test-writer`:
> add a `TEST_CASE` for a full board that still contains a mergeable pair and
> assert `isGameOver() == false`. Run it and watch it fail — `isGameOver()` in
> `src/board.cpp` (lines 122–133) only checks for an empty cell. Fix
> `isGameOver()` so a board with equal orthogonal neighbours is not game over,
> then have `cpp-reviewer` check the diff."*

**Starter prompt — bug B (changed flag).**
> *"In `src/game.cpp`, `Game::run` computes `const bool changed =
> board_.move(toDirection(cmd));` (line 31) but then calls `board_.spawnRandom();`
> unconditionally (line 34), so a no-op move still spawns a tile and `changed` is
> unused. Guard the spawn with `if (changed)`."*

**Walkthrough (full solution).**

*Warm-up — fix the two bugs first.*

A. **`isGameOver()`** (`src/board.cpp`) — after the empty-cell scan, also check
   for equal orthogonal neighbours before declaring the game over:

```cpp
bool Board::isGameOver() const {
    // Any empty cell means a move is still possible.
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            if (grid_[r][c] == 0) {
                return false;
            }
        }
    }
    // A full board is still playable if two orthogonal neighbours are equal.
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            const int v = grid_[r][c];
            if (c + 1 < kSize && grid_[r][c + 1] == v) {
                return false;
            }
            if (r + 1 < kSize && grid_[r + 1][c] == v) {
                return false;
            }
        }
    }
    return true;
}
```

   Turn the scaffold TODO at `tests/test_board.cpp:105` into the test that proves
   it (the `board-test-writer` task):

```cpp
TEST_CASE("a full board with a mergeable pair is not game over") {
    Board board(Grid{{{2, 4, 2, 4}, {4, 2, 4, 2}, {2, 4, 2, 4}, {4, 2, 2, 4}}});
    REQUIRE_FALSE(board.isGameOver());
}
```

B. **The `changed` flag** (`src/game.cpp`) — only spawn when the move actually
   changed the board:

```cpp
        const bool changed = board_.move(toDirection(cmd));
        if (changed) {
            board_.spawnRandom();
        }
        renderer_.draw(board_);
```

*Now the undo feature.* `game-explorer` will point you at `Board::move` (where a
move is applied) as the place to snapshot state.

1. **History storage** (`src/board.hpp`) — add the includes, the `undo()`
   declaration, and a history member:

```cpp
#include <array>
#include <random>
#include <utility>
#include <vector>
```

```cpp
    // Reverts to the state before the last board-changing move. Returns false
    // when there is nothing left to undo.
    bool undo();
```

```cpp
private:
    Grid grid_{};
    int score_ = 0;
    std::mt19937 rng_;

    // Snapshots (grid + score) captured before each board-changing move.
    std::vector<std::pair<Grid, int>> history_;
```

2. **Snapshot on change, and revert** (`src/board.cpp`) — capture the score too,
   push only when the board changed, and add `undo()`:

```cpp
bool Board::move(Direction dir) {
    const Grid before = grid_;
    const int scoreBefore = score_;
    int gained = 0;
```

```cpp
    score_ += gained;
    const bool changed = grid_ != before;
    if (changed) {
        history_.push_back({before, scoreBefore});
    }
    return changed;
}

bool Board::undo() {
    if (history_.empty()) {
        return false;
    }
    const auto [grid, score] = history_.back();
    history_.pop_back();
    grid_ = grid;
    score_ = score;
    return true;
}
```

3. **A key for undo** — add the command (`src/input.hpp`) and map `u`
   (`src/input.cpp`):

```cpp
enum class Command { None, Up, Down, Left, Right, Undo, Quit };
```

```cpp
        case 'u':
        case 'U':
            return Command::Undo;
```

4. **Handle it in the loop** (`src/game.cpp`) — right after the
   `if (cmd == Command::None)` block, undo, redraw, and skip the spawn:

```cpp
        if (cmd == Command::Undo) {
            board_.undo();
            renderer_.draw(board_);
            continue;
        }
```

5. **Test the behaviour** (`tests/test_board.cpp`) — `board-test-writer`'s job:

```cpp
TEST_CASE("undo restores the grid and score before the last move") {
    Board board(Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    board.move(Direction::Left);  // -> {4, 0, 0, 0}, score +4
    REQUIRE(board.score() == 4);

    REQUIRE(board.undo());
    REQUIRE(board.grid() ==
            Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    REQUIRE(board.score() == 0);
}

TEST_CASE("undo with no history returns false") {
    Board board(Grid{{{2, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    REQUIRE_FALSE(board.undo());
}
```

   Finally, hand the diff to `cpp-reviewer` for a read-only pass before you
   commit.

**Done check.** New undo tests pass and the `isGameOver` test (the
`tests/test_board.cpp:105` scaffold) goes green. The changed-flag fix has **no
unit test** — it lives in `Game::run()`, outside the test harness — so verify it
by hand: the unused-variable warning for `changed` is gone and a no-op move no
longer spawns a tile. `cpp-reviewer` reports no blockers. Build + `ctest` green.

## 6. Workflow 🔴 — an AI auto-solver chosen by a judge panel

**Goal.** Add an AI that plays 2048, built and selected by a [workflow](06-workflows.md).
Implement several heuristics — corner-stacking, greedy-merge, monotonicity — then
write a workflow that **generates/benchmarks each heuristic in parallel over N
games** and uses a **judge panel** to pick the winner. (Simpler variant: a
fan-out "configurable board size" refactor where each agent updates one file to
make `kSize` a parameter.)

**Mechanism it teaches.** Deterministic [multi-agent orchestration](06-workflows.md):
`parallel()` fan-out, a benchmark stage, a judge/reduce stage — the
generate-and-select pattern.

**Files.** `src/ai.{hpp,cpp}` (new — keep the solver pure, like `Board`),
`src/board.{hpp,cpp}` (a headless "apply a move" path for self-play),
`.claude/workflows/solver-benchmark.js` (the new solver-selection workflow).

**Inline context.** `.claude/workflows/test-coverage-audit.js` is a complete,
working example of the exact shape you need: a `meta` block with phases, a
`parallel(...)` fan-out, per-agent structured-output schemas, and a final
reduce stage. Use it as your starting model — copy its structure and swap the
read-only `Explore` agents for the build/benchmark steps.

**Starter prompt.**
> *"Use `.claude/workflows/test-coverage-audit.js` as the template (copy its
> `meta`/`phase`/`agent`/structured-schema shape). Write a new workflow
> `.claude/workflows/solver-benchmark.js` that builds several 2048 heuristics —
> corner-stacking, greedy-merge, monotonicity — runs them in parallel with
> `parallel()` over N self-play games, then adds a judge/reduce stage that ranks
> them and picks a winner."*

**Walkthrough (full solution).**

1. **Declare the solver** (`src/ai.hpp`) — pure, like `Board`, no I/O:

```cpp
#pragma once

#include "board.hpp"

namespace g2048 {

// The heuristics the auto-solver can play with.
enum class Heuristic { CornerStacking, GreedyMerge, Monotonicity };

// Scores a grid under a heuristic; higher is better.
int evaluate(const Grid& grid, Heuristic h);

// Picks the legal move the heuristic rates highest. Pure: it copies the board
// to look one move ahead and performs no I/O.
Direction chooseMove(const Board& board, Heuristic h);

// Plays one headless self-play game with the heuristic and returns the final
// score. Spawns tiles exactly like the interactive game.
int playGame(Heuristic h);

}  // namespace g2048
```

2. **Implement the heuristics and self-play** (`src/ai.cpp`). `chooseMove` tries
   each direction on a *copy* of the board and keeps the best legal one;
   `playGame` drives a whole game headlessly:

```cpp
#include "ai.hpp"

namespace g2048 {

namespace {

int sumTiles(const Grid& grid) {
    int sum = 0;
    for (const auto& row : grid) {
        for (int v : row) {
            sum += v;
        }
    }
    return sum;
}

int countEmpty(const Grid& grid) {
    int empty = 0;
    for (const auto& row : grid) {
        for (int v : row) {
            if (v == 0) {
                ++empty;
            }
        }
    }
    return empty;
}

// Reward big tiles pinned to one corner (the classic 2048 strategy).
int cornerScore(const Grid& grid) {
    static const int weight[kSize][kSize] = {
        {15, 14, 13, 12},
        {8, 9, 10, 11},
        {7, 6, 5, 4},
        {0, 1, 2, 3},
    };
    int score = 0;
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            score += grid[r][c] * weight[r][c];
        }
    }
    return score;
}

// Reward keeping the board empty (more room == more future merges).
int greedyMergeScore(const Grid& grid) {
    return countEmpty(grid) * 100 + sumTiles(grid);
}

// Reward rows and columns that stay ordered, so equal tiles line up to merge.
int monotonicityScore(const Grid& grid) {
    int ordered = 0;
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c + 1 < kSize; ++c) {
            if (grid[r][c] >= grid[r][c + 1]) {
                ++ordered;
            }
        }
    }
    for (int c = 0; c < kSize; ++c) {
        for (int r = 0; r + 1 < kSize; ++r) {
            if (grid[r][c] >= grid[r + 1][c]) {
                ++ordered;
            }
        }
    }
    return ordered * 100 + countEmpty(grid) * 10;
}

}  // namespace

int evaluate(const Grid& grid, Heuristic h) {
    switch (h) {
        case Heuristic::CornerStacking:
            return cornerScore(grid);
        case Heuristic::GreedyMerge:
            return greedyMergeScore(grid);
        case Heuristic::Monotonicity:
            return monotonicityScore(grid);
    }
    return 0;
}

Direction chooseMove(const Board& board, Heuristic h) {
    Direction best = Direction::Left;
    int bestScore = 0;
    bool found = false;
    for (Direction dir :
         {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
        Board copy = board;
        if (!copy.move(dir)) {
            continue;  // illegal: the board did not change
        }
        const int score = evaluate(copy.grid(), h);
        if (!found || score > bestScore) {
            found = true;
            bestScore = score;
            best = dir;
        }
    }
    return best;
}

int playGame(Heuristic h) {
    Board board;
    while (!board.isGameOver()) {
        const Direction dir = chooseMove(board, h);
        if (!board.move(dir)) {
            break;  // no legal move improves anything
        }
        board.spawnRandom();
    }
    return board.score();
}

}  // namespace g2048
```

3. **A headless benchmark binary** (`src/bench.cpp`) the workflow can shell out
   to:

```cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ai.hpp"

using namespace g2048;

// Headless benchmark: plays N self-play games with one heuristic and prints
// the average and best score. Usage: ./2048-bench <heuristic> <games>
//   heuristic = corner-stacking | greedy-merge | monotonicity
int main(int argc, char** argv) {
    Heuristic h = Heuristic::CornerStacking;
    int games = 50;

    if (argc > 1) {
        if (std::strcmp(argv[1], "greedy-merge") == 0) {
            h = Heuristic::GreedyMerge;
        } else if (std::strcmp(argv[1], "monotonicity") == 0) {
            h = Heuristic::Monotonicity;
        }
    }
    if (argc > 2) {
        games = std::atoi(argv[2]);
    }

    long total = 0;
    int best = 0;
    for (int i = 0; i < games; ++i) {
        const int score = playGame(h);
        total += score;
        if (score > best) {
            best = score;
        }
    }

    std::printf("heuristic=%s games=%d avg=%.1f best=%d\n",
                argc > 1 ? argv[1] : "corner-stacking", games,
                games > 0 ? double(total) / games : 0.0, best);
    return 0;
}
```

4. **Wire the new files into the build** (`CMakeLists.txt`) — add `src/ai.cpp` to
   `game_core` and a `2048-bench` executable:

```cmake
add_library(game_core
    src/ai.cpp
    src/board.cpp
    src/renderer.cpp
    src/input.cpp
    src/game.cpp)
target_include_directories(game_core PUBLIC src)

# The playable executable.
add_executable(2048 src/main.cpp)
target_link_libraries(2048 PRIVATE game_core)

# Headless benchmark harness for the auto-solver heuristics.
add_executable(2048-bench src/bench.cpp)
target_link_libraries(2048-bench PRIVATE game_core)
```

5. **Test the pure solver logic** — add `tests/test_ai.cpp` and register it in
   `tests/CMakeLists.txt` (`add_executable(unit_tests test_board.cpp test_ai.cpp)`):

```cpp
#include <catch2/catch_test_macros.hpp>

#include "ai.hpp"

using namespace g2048;

TEST_CASE("greedy-merge prefers an emptier board") {
    Grid full{{{2, 4, 2, 4}, {4, 2, 4, 2}, {2, 4, 2, 4}, {4, 2, 4, 2}}};
    Grid sparse{{{2, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};
    REQUIRE(evaluate(sparse, Heuristic::GreedyMerge) >
            evaluate(full, Heuristic::GreedyMerge));
}

TEST_CASE("chooseMove returns a legal move for a playable board") {
    Board board(Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    const Direction dir = chooseMove(board, Heuristic::GreedyMerge);
    REQUIRE(board.move(dir));  // the chosen move actually changes the board
}
```

6. **The selection workflow** (`.claude/workflows/solver-benchmark.js`) — copied
   from `test-coverage-audit.js`'s shape: a `meta` block, a `parallel()` fan-out
   with per-agent schemas, and a judge/reduce stage:

```js
export const meta = {
  name: 'solver-benchmark',
  description:
    'Build the 2048 auto-solver, benchmark each heuristic over N self-play games, and judge-pick the strongest.',
  phases: [
    { title: 'Build', detail: 'configure + build the 2048-bench harness once' },
    { title: 'Benchmark', detail: 'one agent per heuristic runs N self-play games' },
    { title: 'Judge', detail: 'rank heuristics by score and pick a winner' },
  ],
}

// The heuristics in src/ai.cpp, with the CLI name 2048-bench expects.
const HEURISTICS = [
  { key: 'corner-stacking', note: 'pin the biggest tile to a corner' },
  { key: 'greedy-merge', note: 'keep the board as empty as possible' },
  { key: 'monotonicity', note: 'keep rows and columns ordered' },
]

const GAMES = 200  // self-play games per heuristic

const RESULT_SCHEMA = {
  type: 'object',
  properties: {
    heuristic: { type: 'string' },
    games: { type: 'number' },
    avg: { type: 'number', description: 'average final score across the games' },
    best: { type: 'number', description: 'best single-game score' },
  },
  required: ['heuristic', 'avg', 'best'],
}

const RANKING_SCHEMA = {
  type: 'object',
  properties: {
    winner: { type: 'string', description: 'the strongest heuristic key' },
    ranking: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          heuristic: { type: 'string' },
          rank: { type: 'number' },
          avg: { type: 'number' },
          rationale: { type: 'string' },
        },
        required: ['heuristic', 'rank', 'avg'],
      },
    },
    markdown: { type: 'string', description: 'the rendered comparison table' },
  },
  required: ['winner', 'ranking', 'markdown'],
}

// ── Phase 1: build the benchmark harness once ──────────────────────────────
phase('Build')
await agent(
  `In this 2048 C++ repo, configure and build so the benchmark binary exists: ` +
    `run "cmake -S . -B build" then "cmake --build build -j". Confirm ` +
    `./build/2048-bench was produced. Report only OK or the first build error.`,
  { label: 'build:bench', phase: 'Build' }
)

// ── Phase 2: benchmark each heuristic in parallel ──────────────────────────
// A barrier (parallel) is correct: the judge needs every result at once.
phase('Benchmark')
const results = await parallel(
  HEURISTICS.map((h) => () =>
    agent(
      `Run "./build/2048-bench ${h.key} ${GAMES}" in this repo. It prints one ` +
        `line like "heuristic=${h.key} games=${GAMES} avg=1823.1 best=3200". ` +
        `Parse and return the heuristic name, games, avg and best as numbers.`,
      { label: `bench:${h.key}`, phase: 'Benchmark', schema: RESULT_SCHEMA }
    )
  )
)
const scores = results.filter(Boolean)

// ── Phase 3: judge the results and pick a winner ───────────────────────────
phase('Judge')
const ranking = await agent(
  `You are judging three 2048 auto-solver heuristics by their benchmark scores. ` +
    `Rank them best-to-worst by average score (break ties with best score), name ` +
    `the winner, and render a Markdown table with columns: rank | heuristic | avg ` +
    `| best. RESULTS:\n${JSON.stringify(scores, null, 2)}`,
  { label: 'judge:rank', phase: 'Judge', schema: RANKING_SCHEMA }
)

log(`Benchmarked ${scores.length} heuristics; winner: ${ranking.winner}.`)
return ranking.markdown
```

   Run it with `/solver-benchmark`. A quick sanity check from the shell —
   `./build/2048-bench greedy-merge 30` — already prints a real
   `avg=… best=…` line per heuristic for the judge to rank.

**Done check.** The auto-solver plays a full game headlessly; the workflow runs
and reports a ranked comparison. Build + `ctest` green, including tests for the
new pure solver logic.

---

Pick any row, paste its starter prompt, and build it. Each one leaves you with a
working feature *and* a mechanism you've actually driven yourself.
