> 🌍 Read this in: **English** | [Česky](07-project-instructions.cs.md)

# Example 7: Path-scoped project rules

## What are project instructions?

**Project instructions** are Markdown files Claude reads as standing orders for
a repository. There are three kinds, and they differ in *when* they load:

- `CLAUDE.md` (or `.claude/CLAUDE.md`) is loaded at the start of **every**
  session. Keep it short: build commands, conventions, layout.
- `.claude/rules/*.md` **without** a `paths:` key also load at session start.
  They're a way to split a long `CLAUDE.md` into topic files.
- `.claude/rules/*.md` **with** a `paths:` key load **only when Claude reads
  or edits a matching file**. That's the one this example teaches.

`AGENTS.md`, the cross-tool instruction file other coding agents use, is read
as well, but by default **only when there is no `CLAUDE.md`** in your working
directory or above it. This repo has a `CLAUDE.md`, so an `AGENTS.md` here
would be ignored unless you change the **Project instructions** setting in
`/config`. (Reading `AGENTS.md` directly needs Claude Code 2.1.277 or later.)

## What this example does

The rule `board-logic` carries the constraints that matter **only when
someone touches the game rules**: `Board` must stay free of I/O, tests must
use the deterministic constructor, `ctest` must run afterwards. A participant
working on the renderer never pays the context cost of these lines. The moment
Claude opens `src/board.cpp` or `src/board.hpp`, they load.

## The file, line by line

The rule lives at `.claude/rules/board-logic.md`:

```
.claude/              ← project-scoped Claude Code config (committed to git)
└── rules/            ← every .md file in here is a rule (searched recursively)
    └── board-logic.md  ← file name is free; only the .md extension matters
```

```markdown
---
paths:
  - "src/board.cpp"
  - "src/board.hpp"
---

# Board logic rules

These rules load only when Claude reads or edits the board files.

- `Board` stays free of I/O: no `<iostream>`, no terminal calls, no
  `std::rand()` seeding inside game rules. Rendering belongs in `renderer.cpp`.
- Keep `slideLineLeft()` a free function so tests can call it directly with a
  `std::array<int, kSize>`.
- Every behaviour change needs a deterministic test that builds the board with
  the `Board(Grid grid, int score)` constructor, never with `spawnRandom()`.
- After any change, run `ctest --test-dir build --output-on-failure` and
  report the result.
```

What each part does:

- `paths:` — a list of glob patterns. **This is the only frontmatter key a
  rule has.** With it, the rule is *path-scoped*: it loads when Claude uses
  Read, Write or Edit on a matching file, not on every tool call. Without
  it, the rule loads at session start like `CLAUDE.md`.
- The globs are relative to the project root. `src/**/*.cpp` or
  `tests/**` work too.
- The body is plain Markdown. Write it like `CLAUDE.md`: short, imperative,
  specific to the files the rule is scoped to.

## Create your own rule, step by step

1. **Create the folder:**
   ```bash
   mkdir -p .claude/rules
   ```

2. **Create a rule file**, e.g. `.claude/rules/tests.md`:
   ```markdown
   ---
   paths:
     - "tests/**"
   ---

   # Test rules

   - One behaviour per TEST_CASE, Arrange-Act-Assert.
   - Never weaken an assertion to make a test pass; report the bug instead.
   ```

3. **Start a new session.** Rules are discovered at session start.

4. **Check that it loads only when it should:** ask a question about
   `src/renderer.cpp` first, then run `/context`. The rule is **not** listed
   under **Memory files**. Now ask Claude to read `tests/test_board.cpp`
   and run `/context` again. The rule is listed.

5. **Commit it:**
   ```bash
   git add .claude/rules/tests.md
   git commit -m "Add path-scoped test rules"
   ```

## Try the demo

Ask Claude: *"Read src/board.hpp. Is a project rule titled 'Board logic
rules' in your context? Quote its first bullet."*

Claude quotes the I/O bullet. Ask the same question in a fresh session
**without** reading a board file first and Claude can't see the rule.

## Audit your instructions: `/doctor prompt-audit`

Instruction files rot. They keep wording written for older models, point at
commands that no longer exist, or contradict each other. Since 2.1.283,
`/doctor prompt-audit` (also `/checkup prompt-audit`) reviews your
`CLAUDE.md`, `CLAUDE.local.md` and `AGENTS.md` files, plus the rules, skills,
commands, subagents and output styles under `.claude/` and `~/.claude/`. You
get a report with proposed edits, and nothing changes until you ask Claude to
apply them. To audit one path only:

```text
/doctor prompt-audit .claude/rules
```

## Optional parameters

| Setting / key | Where | What it does |
|---------------|-------|--------------|
| `paths:` | rule frontmatter | Glob list. Makes the rule path-scoped |
| `@path/to/file` | inside `CLAUDE.md` | Imports another file into `CLAUDE.md`. Relative paths resolve from the importing file. Max 4 hops |
| `claudeMdExcludes` | any `settings.json` | Glob list of absolute paths to skip, e.g. another team's `CLAUDE.md` in a monorepo. Managed policy files can't be excluded |
| **Project instructions** | `/config` | Choose whether to read `CLAUDE.md` only, `AGENTS.md` too, or managed instructions only |
| `omitClaudeMd: true` | subagent frontmatter | The subagent skips user, project and local `CLAUDE.md` files. Built-in Explore and Plan already do |

Full reference: [official memory documentation](https://code.claude.com/docs/en/memory).

## Where it works: CLI, Desktop app, Cowork

| Platform | Works? | Setup |
|----------|--------|-------|
| **Claude Code CLI** (terminal) | ✅ Yes | Nothing extra. Rules in `.claude/rules/` are discovered at session start |
| **Claude Desktop app — Code tab** | ✅ Yes | Same engine as the CLI. Open the project folder and confirm the trust dialog |
| **Cowork** (in the Desktop app) | ❌ No | Cowork doesn't load project-scoped `.claude/` config. Put the instructions in a skill inside a plugin instead |

## Troubleshooting

- **Rule never shows up in `/context`** → check the glob against the real
  path (`src/board.cpp`, not `./src/board.cpp`). Remember that a path-scoped
  rule appears only **after** Claude reads or edits a matching file.
- **Rule loads in every session** → the frontmatter is missing or broken, so
  there is no `paths:` key. The `---` lines must be the first line of the file.
- **`AGENTS.md` is ignored** → expected while a `CLAUDE.md` exists. Either
  import it from `CLAUDE.md` with `@AGENTS.md`, or change **Project
  instructions** in `/config`.
