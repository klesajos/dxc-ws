> 🌍 Read this in: **English** | [Česky](exercises.cs.md)

# Exercise catalog: one feature per mechanism

You've worked through all six extension mechanisms ([Skills](01-skills.md),
[Hooks](02-hooks.md), [MCP](03-mcp.md), [Plugins](04-plugins.md),
[Subagents](05-agents.md), [Workflows](06-workflows.md)). This catalog turns
them into practice: **each game feature below is a vehicle for exercising one
specific mechanism.** The point isn't only the feature — it's getting the
mechanism into your hands.

Every exercise states its **goal**, the **mechanism** it teaches, the **files**
you'll touch, and a **done check**. The done check is always the same baseline:
the project still builds and the suite is green —

```bash
cmake -S . -B build && cmake --build build -j && ctest --test-dir build --output-on-failure
```

**Easiest on-ramp:** this repo ships with two real, deliberate bugs. Fixing them
needs almost no new code and is the fastest way to feel a mechanism work:

- **`isGameOver()`** (`src/board.cpp`) only checks for an empty cell, so a full
  board that still has a mergeable pair is wrongly reported as game over.
- **The `changed` flag** in `src/game.cpp` is computed from `board_.move(...)`
  and then ignored, so a tile spawns even after a move that changed nothing.

Difficulty: 🟢 Easy · 🟡 Medium · 🔴 Hard.

## At a glance

| Mechanism | Feature exercise | Difficulty | Touches |
|---|---|---|---|
| **Skill** | A `renderer-style` skill, then colour tiles + a move counter | 🟢–🟡 | `renderer.cpp`, `board.{hpp,cpp}` |
| **Hook** | A `ctest` pre-commit/Stop hook, then "configurable win value" behind it | 🟡 | `.claude/settings.json`, `board.{hpp,cpp}` |
| **MCP** | High-score persistence via the `memory` MCP server | 🟡 | `game.cpp`, `.mcp.json` |
| **Plugin** | Package undo + replay as `/2048-dev:` commands | 🟡 | `plugins/2048-dev/` |
| **Subagent** | Undo feature: planned, tested, reviewed by the agents; fix the two bugs | 🟢–🟡 | `board`, `game`, `input` |
| **Workflow** | AI auto-solver: parallel heuristics, benchmarked, judge-picked | 🔴 | new `ai.{hpp,cpp}`, `board` |

---

## 1. Skill — teach the renderer's conventions, then use them

**Goal.** Write a new skill `renderer-style` that captures this project's ANSI /
terminal-drawing conventions (the escape codes in `renderer.cpp`, the
`kCellWidth` alignment, the score header). Then, *with the skill active*, add
**colour tiles** (different ANSI colour per tile value) and a **move counter**
shown in the header.

**Mechanism it teaches.** Authoring a [skill](01-skills.md) — a description that
triggers on the right requests, a body that briefs Claude on conventions — and
then watching it load on demand. Contrast with `board-tests`: same shape, new
domain.

**Files.** `.claude/skills/renderer-style/SKILL.md` (new), `src/renderer.cpp`,
`src/board.{hpp,cpp}` (a move count lives on the game state).

**Done check.** Build + `ctest` green; the game renders coloured tiles and a
move counter. (Keep rendering out of `Board` — colours belong to `Renderer`.)

## 2. Hook — make tests un-skippable, then add a gated feature

**Goal.** Add a hook that runs `ctest` and **blocks on red** — either a `Stop`
hook (so a session can't end with failing tests) or a pre-commit hook. Then build
a **configurable win value** (e.g. play to 1024 or 4096 instead of 2048) and let
the hook guarantee you never commit it with a broken suite.

**Mechanism it teaches.** A [hook](02-hooks.md) that enforces something *every
time*, with no model discretion — the complement to the existing format hook.

**Files.** `.claude/settings.json` (+ a script under `.claude/hooks/`),
`src/board.{hpp,cpp}` (`kWinValue` becomes configurable).

**Done check.** Deliberately break a test → the hook blocks. Fix it → it passes.
Build + `ctest` green with the new win value.

## 3. MCP — persist the high score across sessions

**Goal.** Use the already-configured **`memory`** MCP server (see `.mcp.json`) to
store and retrieve the best score, so it survives across runs. Claude reads/writes
the high score through the MCP server; the game displays "best: N".

**Mechanism it teaches.** Giving Claude a [capability](03-mcp.md) it doesn't have
natively (durable memory) via an MCP server — and wiring its data into the app.

**Files.** `src/game.cpp` (surface the best score), `.mcp.json` (already has
`memory`; confirm it's connected with `/mcp`).

**Done check.** Play a game, reach a score, start a new session → the best score
persists. Build + `ctest` green.

## 4. Plugin — package undo + replay as shareable commands

**Goal.** Extend the `2048-dev` plugin with developer commands:
`/2048-dev:undo` and `/2048-dev:replay` (a command that scaffolds or drives an
undo/replay feature), plus a **feature-scaffolding** command that stubs a new
`Board` method with a matching test.

**Mechanism it teaches.** Growing a [plugin](04-plugins.md) — more commands in
`plugins/2048-dev/commands/`, a version bump, validation — so a whole team shares
the same workflow commands.

**Files.** `plugins/2048-dev/commands/*.md` (new), `plugins/2048-dev/.claude-plugin/plugin.json`
(version bump). Validate with `claude plugin validate .`.

**Done check.** `claude plugin validate .` passes; the new commands autocomplete
and run. Build + `ctest` green.

## 5. Subagent — delegate an undo feature across the three agents

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
undo command), `src/input.cpp` (map a key to it).

**Done check.** New tests for undo pass; both known-bug tests pass; `cpp-reviewer`
reports no blockers. Build + `ctest` green.

## 6. Workflow — an AI auto-solver chosen by a judge panel

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
`.claude/workflows/*.js` (the solver-selection workflow).

**Done check.** The auto-solver plays a full game headlessly; the workflow runs
and reports a ranked comparison. Build + `ctest` green, including tests for the
new pure solver logic.

---

Pick any row, read the matching guide, and build it. Each one leaves you with a
working feature *and* a mechanism you've actually driven yourself.
