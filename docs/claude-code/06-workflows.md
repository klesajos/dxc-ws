> 🌍 Read this in: **English** | [Česky](06-workflows.cs.md)

# Example 6: Workflows (multi-agent orchestration)

## What is a workflow?

A **workflow** is a small JavaScript program that **orchestrates many agents
with deterministic control flow**. Instead of asking Claude to "figure out the
steps", you write the steps — fan out work with `parallel()`, chain it with
`pipeline()`, group it with `phase()` — and the runtime executes that structure
exactly, every time. A saved workflow becomes a slash command you can run with
`/<name>`.

Where a single agent decides its own path, a workflow's path is fixed in code:
loops, conditionals, and fan-out are yours to control.

## Subagent vs. workflow

These are the two delegation mechanisms, and it's worth being clear on the
difference before reading the code:

| | Subagent (Example 5) | Workflow (this example) |
|---|---|---|
| **Unit** | one delegated context | many agents, orchestrated |
| **Control flow** | the agent decides its own steps | *you* decide, in JavaScript (`parallel`/`pipeline`/`phase`) |
| **Determinism** | model-driven, varies run to run | the structure is fixed; same input → same shape of run |
| **Defined in** | a Markdown file + system prompt | a `.js` script with `export const meta` |
| **Invoke** | plain language, `@agent-name`, `--agent` | a slash command `/<name>` (opt-in) |

**Rule of thumb:** reach for a **subagent** when you want to hand *one*
self-contained task to *one* isolated context. Reach for a **workflow** when you
want to run *many* agents in a *repeatable, code-defined* pipeline — fan out over
N files, gate stage 2 on stage 1, reduce many results into one report.

## What this example does

This example ships one runnable workflow, `test-coverage-audit`, that audits how
well `tests/test_board.cpp` covers the logic in `src/`. It is **read-only,
deterministic, and re-runnable** — it prints a prioritized gap report and never
writes a file, so you can run it as often as you like and get a stable result.

It works in three phases:

1. **Inventory** — fan out one read-only reader per source file; each enumerates
   the behaviours a unit test could target.
2. **Map tests** — one agent maps every existing `TEST_CASE` onto that inventory.
3. **Report gaps** — one reducer cross-references the two and emits a prioritized
   gap report.

Because the real gaps in this repo are known (the `{4, 4, 8, 0}` TODO, the
`spawnRandom` distribution, the full-board-with-a-mergeable-pair game-over case
that trips the `isGameOver()` bug, multi-move sequences, the ignored `changed`
flag in `game.cpp`), the report is predictable — which makes it a good teaching
artifact.

## The file, line by line

The workflow lives at `.claude/workflows/test-coverage-audit.js`. Saved
workflows in `.claude/workflows/` are auto-discovered and become slash commands.

**1. The `meta` block must be the first statement.** It names the workflow (this
becomes `/test-coverage-audit`) and describes its phases:

```js
export const meta = {
  name: 'test-coverage-audit',
  description: 'Audit src/ test coverage against tests/test_board.cpp and emit a prioritized gap report.',
  phases: [
    { title: 'Inventory', detail: 'one read-only reader per src file enumerates behaviours' },
    { title: 'Map tests', detail: 'map each TEST_CASE onto the behaviour inventory' },
    { title: 'Report gaps', detail: 'cross-reference and emit a prioritized gap report' },
  ],
}
```

**2. Schemas force each agent to return data, not prose.** Passing a `schema` to
`agent()` makes the runtime validate the agent's output, so the script gets a
real object back — no parsing:

```js
const BEHAVIOR_SCHEMA = {
  type: 'object',
  properties: {
    behaviors: { type: 'array', items: { type: 'object', properties: {
      id: { type: 'string' }, file: { type: 'string' }, line: { type: 'number' },
      behavior: { type: 'string' }, pureLogic: { type: 'boolean' }, sideEffects: { type: 'string' },
    }, required: ['id', 'file', 'line', 'behavior', 'pureLogic'] } },
  },
  required: ['behaviors'],
}
```

**3. Phase 1 fans out with `parallel()` — and every agent is read-only.** Each
reader runs as the built-in **`Explore`** agent type (`agentType: 'Explore'`),
which can read and search but **cannot** `Edit`, `Write`, or build. That is how
the whole workflow is guaranteed side-effect-free:

```js
phase('Inventory')
const inventories = await parallel(
  SRC_FILES.map((f) => () =>
    agent(`Read ${f.file} ... enumerate every behaviour a unit test could target ...`,
      { label: `inventory:${f.file}`, phase: 'Inventory', schema: BEHAVIOR_SCHEMA, agentType: 'Explore' })
  )
)
const behaviors = inventories.filter(Boolean).flatMap((r) => r.behaviors)
```

`parallel()` is a **barrier**: it waits for *all* readers before continuing.
That's deliberate here — Phase 2 needs the *whole* inventory before it can map
tests onto it. (Contrast `pipeline()`, which runs each *item* through all stages
independently; it suits per-item chains like the feature-pipeline below, not a
fan-in.)

**4. Phases 2 and 3 run sequentially** because each depends on the previous
phase's aggregated result — `phase()` just labels the group in the progress UI:

```js
phase('Map tests')
const coverageResult = await agent(`Read tests/test_board.cpp ... map each TEST_CASE ...`,
  { label: 'map:tests', schema: COVERAGE_SCHEMA, agentType: 'Explore' })

phase('Report gaps')
const report = await agent(`Cross-reference behaviours vs coverage; emit a prioritized gap report ...`,
  { label: 'reduce:gaps', schema: GAP_SCHEMA, agentType: 'Explore' })

log(`Audited ${behaviors.length} behaviours; found ${report.gaps.length} coverage gaps.`)
return report.markdown   // the script's return value is what the command surfaces
```

The script's `return` value is what `/test-coverage-audit` shows you — here, the
rendered Markdown report.

## A more advanced shape: the feature-pipeline (described, not shipped)

The audit only reads. A workflow can also *do* work safely if you isolate it.
A `feature-pipeline` for adding a feature would look like:

```
design (read-only architect)
  → enter a git worktree (isolation)
    → implement the feature
    → cmake + ctest, with a bounded retry on failure
    → review with the cpp-reviewer agent from Example 5   ← agents compose into workflows
  → exit the worktree
→ hand the branch to a human to merge
```

Two ideas worth noting: a **worktree** keeps the work on an isolated copy of the
repo so a failed run never touches your tree, and the review stage **reuses the
`cpp-reviewer` subagent** — Example 5's agent becomes a stage in Example 6's
pipeline. It's worth this much machinery when a change is multi-file, test-gated,
and you want concurrency; it's overkill for a one-line fix you'd just make
directly.

## Create your own workflow, step by step

1. **Create the folder:**
   ```bash
   mkdir -p .claude/workflows
   ```
2. **Create** `.claude/workflows/my-flow.js` starting with `meta`:
   ```js
   export const meta = { name: 'my-flow', description: 'What it does.', phases: [{ title: 'Work' }] }
   phase('Work')
   const result = await agent('Do one well-scoped thing.', { schema: { type: 'object', properties: { summary: { type: 'string' } }, required: ['summary'] } })
   return result.summary
   ```
3. **Keep it deterministic and, ideally, read-only first.** Use
   `agentType: 'Explore'` for agents that should only read. Add write power only
   when you also add isolation (a worktree).
4. **Restart Claude Code** so the workflow is discovered, then run `/my-flow`.
   Running a workflow is **opt-in** — you invoke it explicitly.
5. **Commit it:**
   ```bash
   git add .claude/workflows/my-flow.js && git commit -m "Add my-flow workflow"
   ```

## Try the demo

1. In a fresh `claude` session in the repo root, run **`/test-coverage-audit`**.
   Watch it fan out over the five `src/` files (Phase 1), map the tests
   (Phase 2), and print a prioritized gap report (Phase 3) that names the known
   gaps — including the `isGameOver()` bug as a latent issue.
2. Check `git status` — it's **clean**. The workflow only read.
3. Run it again. The *shape* is the same and the same gaps surface — that's the
   determinism a code-defined pipeline buys you.
4. Read the "feature-pipeline" section above and trace where a worktree would
   isolate the work and where `cpp-reviewer` plugs in as the review stage.

## Optional parameters

Per-`agent()` options you'll reach for most:

| Option | What it does |
|--------|--------------|
| `schema` | JSON Schema the agent's output must match; `agent()` returns the validated object |
| `label` | The name shown for this agent in the progress UI |
| `phase` | Assigns the agent to a progress group (use inside `parallel`/`pipeline`) |
| `agentType` | Run as a specific agent type, e.g. `Explore` for enforced read-only |
| `model` / `effort` | Override model or reasoning effort for a stage |
| `isolation: 'worktree'` | Run the agent in a fresh git worktree — for stages that mutate files |

Orchestration primitives: `phase()`, `parallel()` (barrier), `pipeline()`
(per-item chains, no barrier), `log()`. Full reference:
[official documentation](https://code.claude.com/docs/en/claude-code-on-the-web).

## Where it works: CLI, Desktop app, Cowork

| Platform | Works? | Setup |
|----------|--------|-------|
| **Claude Code CLI** (terminal) | ✅ Yes | Workflows in `.claude/workflows/` are discovered automatically; run `/test-coverage-audit` |
| **Claude Desktop app — Code tab** | ✅ Yes* | Same engine as the CLI; the project's workflows load after the trust dialog. *Verify the workflows feature is enabled in your build |
| **Cowork** (in the Desktop app) | ❌ No* | Cowork does not load project-scoped `.claude/` config, including `.claude/workflows/`. *Workflow orchestration is a local engine feature; package the underlying agents as a plugin if you need them in Cowork |

## Troubleshooting

- **`/test-coverage-audit` doesn't appear** → the file must be in
  `.claude/workflows/` and start with `export const meta = { ... }` as its first
  statement; restart the session.
- **An agent errors with "unknown agent type"** → your build may not expose the
  `Explore` type; remove `agentType: 'Explore'` and instead instruct the agent to
  read only (less strongly enforced, but it runs).
- **The report changes a lot between runs** → tighten the reducer prompt and
  schema; the more structure you pin down, the more stable the output.

---

You've now seen all six project-scoped mechanisms. For where to go next — a
graded menu of game features, each a vehicle to practise one mechanism — see the
[exercise catalog](exercises.md).
