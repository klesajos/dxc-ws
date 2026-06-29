> 🌍 Read this in: **English** | [Česky](05-agents.cs.md)

# Example 5: Subagents (custom agents)

## What is a subagent?

A **subagent** is a specialised assistant that runs in its **own, isolated
context** with its **own tools and persona**. When the main conversation hands
it a task, the subagent works it through separately and reports back a result —
its intermediate steps never clutter your main context.

Two things make subagents distinct from the earlier examples:

- **Context isolation.** A skill (Example 1) loads instructions *into your
  conversation*. A subagent runs in a *separate* conversation. Useful when a job
  is self-contained ("review this diff", "write this test") and you only want the
  conclusion back.
- **Tool restriction / persona.** You can give a subagent *fewer* tools than you
  have. A reviewer with no `Edit`/`Write` **physically cannot** modify files — the
  restriction is enforced, not merely requested.

Rule of thumb: a **skill** teaches Claude *how* to do something in your current
context; a **subagent** is *who* you delegate a whole task to.

## What this example does

This example ships **three** subagents that deliberately span the spectrum:

| Agent | Lives in | Tools | Why it's here |
|-------|----------|-------|---------------|
| `board-test-writer` | `.claude/agents/` (project) | read **+ write + Bash** | Write-capable; **preloads the `board-tests` skill** — shows subagents compose with Example 1 |
| `cpp-reviewer` | `.claude/agents/` (project) | read-only (no `Edit`/`Write`) | The tool-restriction teaching artifact: a reviewer that cannot modify code |
| `game-explorer` | `plugins/2048-dev/agents/` (plugin) | read-only, no `Bash` | Bundled with the plugin from Example 4; auto-discovered and namespaced `2048-dev:game-explorer` |

Together they show the same mechanism at three points: project-scoped and
write-capable, project-scoped and locked down, and plugin-bundled.

## The files, line by line

A project subagent is a single Markdown file at `.claude/agents/<name>.md`:

```
.claude/              ← project-scoped Claude Code config (committed to git)
└── agents/           ← all project subagents live here
    ├── board-test-writer.md
    └── cpp-reviewer.md
```

`cpp-reviewer` is the clearest one to read — the tool restriction *is* the lesson.
Here is its frontmatter, **verbatim** from `.claude/agents/cpp-reviewer.md`:

```yaml
---
name: cpp-reviewer
description: |
  Use to review C++ changes in this repo for correctness and house style before
  you commit. Reads the diff and the surrounding code and reports severity-tagged
  findings. It has no Edit or Write access — it reviews, it never modifies. Pairs
  with the clang-format hook: the hook handles mechanical formatting, this agent
  handles everything formatting cannot catch.

  <example>
  Context: the user just edited src/game.cpp and src/board.cpp and wants a second opinion.
  user: "Review my uncommitted changes."
  assistant: "I'll run the cpp-reviewer agent — it reads the diff and reports Blocker/Should-fix/Nit findings with file:line, and modifies nothing."
  <commentary>Read-only semantic review is exactly this agent's role; git status stays unchanged afterwards.</commentary>
  </example>

  <example>
  Context: before opening a PR.
  user: "Anything wrong with this branch before I push?"
  assistant: "Delegating to cpp-reviewer to check the diff against the project conventions in CLAUDE.md."
  <commentary>The agent checks naming, const-correctness, ownership and off-by-ones — the things .clang-format can't see.</commentary>
  </example>
tools: Read, Grep, Glob, Bash(git diff:*), Bash(git status:*), Bash(git log:*)
model: inherit
color: blue
---
```

What each part does:

- `name` — the agent's identifier. You address it as `@agent-cpp-reviewer`, by
  asking in plain language ("review my changes"), or with `claude --agent cpp-reviewer`.
- `description` — **the most important field.** It's what Claude reads to decide
  *when to delegate to this agent on its own*. Write it as a trigger, and add
  `<example>` / `<commentary>` blocks: each example is a mini scenario that teaches
  auto-delegation ("when the user says *review my changes*, use me").
- `tools` — the **allowlist**. `Read, Grep, Glob` plus a **scoped** `Bash`
  (`Bash(git diff:*)`, `Bash(git status:*)`, `Bash(git log:*)`) and **no
  `Edit`/`Write`** means this agent can read the repo and inspect the diff, but
  cannot run an arbitrary command or change a single file. Scoping `Bash(cmd:*)`
  is tighter than granting bare `Bash`. Omit the field entirely to inherit *all*
  tools; list it to lock the agent down. (You can also use `disallowedTools` to
  subtract from the default set.)
- `model: inherit` — use the same model as the conversation that spawned it.
- `color` — a display colour in the agents UI.
- Everything **below** the frontmatter is the agent's **system prompt** — its
  persona and instructions, followed only inside its own context.

### The system prompt: where the read-only guarantee becomes visible

The body is where the read-only nature stops being a promise and becomes something
you can *see*. `cpp-reviewer` ends its system prompt with a fixed `## Output
format` — quoted here **verbatim**:

```markdown
## Output format

Group findings by severity, most serious first:

- **Blocker** — wrong behaviour or a build break.
- **Should-fix** — a real problem that is not strictly breaking.
- **Nit** — style/clarity, optional.

Each finding: `path/file.cpp:line` — one sentence on what is wrong and one on the
fix. If you find nothing in a category, say so. End with the line:

> No files modified — review only.
```

That last line is the demo's proof. When you ask *"Review my uncommitted changes,"*
you should see this exact footer — `> No files modified — review only.` — and an
**unchanged `git status`**. The frontmatter *withholds* `Edit`/`Write`; the system
prompt makes the consequence visible in plain sight.

The other two add one idea each.

**`board-test-writer.md`** carries **`skills: board-tests`** in its frontmatter.
That **preloads the Example 1 skill** into the agent, so it inherits the test
conventions without copying them — subagent and skill composing layer on layer.
Its allowlist is the full **`tools: Read, Edit, Write, Grep, Glob, Bash`** set: the
read tools, plus `Edit`/`Write` to add a `TEST_CASE` and `Bash` to run `ctest`.

```yaml
---
name: board-test-writer
description: |
  Use to write or extend Catch2 unit tests for the 2048 board logic in
  tests/test_board.cpp. ...
tools: Read, Edit, Write, Grep, Glob, Bash
model: inherit
color: green
skills: board-tests
---
```

**`plugins/2048-dev/agents/game-explorer.md`** lives **inside the plugin**, in an
`agents/` folder **next to** `commands/` (see Example 4, which already lists
`agents/` as a valid plugin folder). Its **`tools: Read, Grep, Glob`** make it
read-only with **no `Bash`**, and it has **no `model:` line** at all. The file
itself says `name: game-explorer`; Claude Code auto-discovers it and namespaces it
as **`2048-dev:game-explorer`** *because it is plugin-bundled*. No settings entry
needed.

```yaml
---
name: game-explorer
description: |
  Use to understand how a feature flows through the 2048 codebase before changing
  it. ...
tools: Read, Grep, Glob
color: cyan
---
```

> **A real capability boundary:** a plugin-bundled agent **cannot** define its own
> `hooks`, `mcpServers`, or `permissionMode` — those belong to the plugin, not to
> an agent inside it. Project agents in `.claude/agents/` and plugin agents share
> the same file format, but the plugin context is the more restricted one.

## Create your own subagent, step by step

1. **Create the folder** (project-scoped):
   ```bash
   mkdir -p .claude/agents
   ```
2. **Create the file** `.claude/agents/my-agent.md`:
   ```markdown
   ---
   name: my-agent
   description: Use when <the situation that should trigger delegation>.
   tools: Read, Grep, Glob
   model: inherit
   ---

   # My agent
   <The system prompt: who this agent is, what it does, how it reports back.>
   ```
3. **Decide its powers.** Read-only? List `Read, Grep, Glob` and stop. Needs to
   edit and build? Add `Edit, Write, Bash`. Omit `tools` only if it truly needs
   everything.
4. **Restart the session — this is the step everyone forgets.** Agents are
   discovered only at session start; adding `.claude/agents/<name>.md` mid-session
   and then typing `@agent-<name>` fails with "agent not found" until you start a
   fresh `claude`.
5. **Invoke it** three ways: ask in plain language matching the `description`,
   mention `@agent-my-agent`, or run `claude --agent my-agent`.
6. **Commit it** so everyone who clones the repo gets it:
   ```bash
   git add .claude/agents/my-agent.md && git commit -m "Add my-agent subagent"
   ```
   (Personal agents you don't want to share go in `~/.claude/agents/` instead.)

## Try the demo

Run these in a fresh `claude` session in the repo root:

1. **`cpp-reviewer` (read-only).** Make a trivial edit to any `.cpp`, then ask:
   *"Review my uncommitted changes."* You get severity-tagged findings with
   `file:line` and a *"No files modified"* footer — and `git status` is unchanged,
   because the agent has no way to write. (Bonus: edit `src/game.cpp` and it should
   flag the ignored `changed` flag.)
2. **`board-test-writer` (write-capable).** Ask: *"Add the test for the line
   {4, 4, 8, 0}."* It loads the `board-tests` conventions, adds a `TEST_CASE`, and
   runs `ctest`. Then ask for the full-board game-over test — confirm it **surfaces
   the `isGameOver()` bug** instead of weakening the assertion to force a pass.
3. **`game-explorer` (plugin-bundled).** Ask: *"Trace how a keypress becomes a
   board move."* You get a numbered `file:line` trace and no edits. It came from the
   plugin — verify with `claude plugin validate .`.

## Optional frontmatter parameters

| Field | What it does |
|-------|--------------|
| `tools` | Allowlist of tools the agent may use. Omit = inherit all; list = lock down (e.g. read-only by omitting `Edit`/`Write`) |
| `disallowedTools` | Subtract specific tools from the default set. Ignored if `tools` is set |
| `model` | Model for this agent; `inherit` matches the spawning conversation |
| `color` | Display colour in the agents UI |
| `skills` | Preload one or more skills into the agent (comma list or YAML array), e.g. `skills: board-tests` |
| `<example>` / `<commentary>` in `description` | Scenario blocks that teach Claude *when* to auto-delegate to this agent |

Full, current list: [official subagents documentation](https://code.claude.com/docs/en/sub-agents).

## Where it works: CLI, Desktop app, Cowork

| Platform | Works? | Setup |
|----------|--------|-------|
| **Claude Code CLI** (terminal) | ✅ Yes | Agents in `.claude/agents/` are discovered automatically when you run `claude` in the repo |
| **Claude Desktop app — Code tab** | ✅ Yes | Same engine as the CLI; the project's `.claude/agents/` load after the one-time trust dialog |
| **Cowork** (in the Desktop app) | ⚠️ Via a plugin | Cowork does **not** load project-scoped `.claude/agents/`. But an agent **bundled in a plugin** (like `game-explorer`) rides along when the plugin is installed in Cowork — so package the agent in a plugin to make it portable |

This is the same pattern as Example 1: project-scoped `.claude/` config doesn't
reach Cowork, but plugin-bundled content does. `game-explorer` is portable
precisely because it lives in the plugin, not in `.claude/agents/`.

## Troubleshooting

- **Agent never gets delegated to automatically** → your `description` is too
  vague, or missing `<example>` blocks. Describe the *user's situation* and add
  scenarios that trigger it.
- **Agent edited a file you expected it not to** → it inherited all tools because
  you omitted `tools`. Add an explicit allowlist without `Edit`/`Write`.
- **`@agent-name` not found / plugin agent missing** → restart the session
  (agents load at session start); for the plugin one, confirm it's enabled and run
  `claude plugin validate .`.

---

Next: subagents delegate **one** task to **one** isolated context. When you want
to orchestrate **many** agents with deterministic, code-defined control flow, you
reach for a workflow → [Example 6: Workflows](06-workflows.md).
