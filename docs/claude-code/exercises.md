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
explained inline, and a **done check**. The linked guide is optional depth.

## How to use this catalog

1. **Pick a row** from the at-a-glance table below.
2. **Open the linked guide** for that mechanism if you want the *how* and *why* —
   otherwise skip it; the exercise stands on its own.
3. **`Shift+Tab`** to switch Claude Code into plan or accept-edits mode (so you
   review or auto-apply edits the way you prefer).
4. **Paste the Starter prompt** from the exercise verbatim.
5. **Run the Done-check command** to confirm it worked.

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

**Done check.** The auto-solver plays a full game headlessly; the workflow runs
and reports a ranked comparison. Build + `ctest` green, including tests for the
new pure solver logic.

---

Pick any row, paste its starter prompt, and build it. Each one leaves you with a
working feature *and* a mechanism you've actually driven yourself.
