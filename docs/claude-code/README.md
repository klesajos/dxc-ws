> 🌍 Read this in: **English** | [Česky](README.cs.md)

# Claude Code extension examples

This repo doubles as a showcase of the six **project-scoped** ways to extend
Claude Code. "Project-scoped" means the configuration lives **inside the
repository** — when you `git clone` the project, you get the whole setup
automatically. Nothing needs to be installed into your home directory.

Each example is independent and minimal. Work through them in order — they
go from simplest to most advanced:

| # | Example | What it teaches | Where it lives |
|---|---------|-----------------|----------------|
| 1 | [Skills](01-skills.md) | Giving Claude reusable project knowledge it loads only when needed | `.claude/skills/board-tests/SKILL.md` |
| 2 | [Hooks](02-hooks.md) | Running a shell command automatically when Claude does something (here: auto-format code) | `.claude/settings.json` + `.claude/hooks/format-cpp.sh` |
| 3 | [MCP](03-mcp.md) | Connecting Claude to external tools and data sources | `.mcp.json` |
| 4 | [Plugins](04-plugins.md) | Packaging commands/skills/hooks so a whole team can share them | `plugins/2048-dev/` + `.claude-plugin/marketplace.json` |
| 5 | [Subagents](05-agents.md) | Delegating a self-contained task to an isolated context with its own tools and persona | `.claude/agents/` + `plugins/2048-dev/agents/` |
| 6 | [Workflows](06-workflows.md) | Orchestrating many agents with deterministic, code-defined control flow | `.claude/workflows/` |

Every guide exists in two languages: `xx-name.md` is English,
`xx-name.cs.md` is Czech. They have the same content. When you've worked
through all six, the [exercise catalog](exercises.md) gives you one game
feature to build per mechanism — and the [cheat-sheet](cheatsheet.md) is a
quick reference for the day-to-day flags, slash commands and shortcuts.

## Before you start

You need:

1. **Claude Code installed** — follow the official
   [setup guide](https://code.claude.com/docs/en/setup) (system requirements and
   install methods) or the [quickstart](https://code.claude.com/docs/en/quickstart)
   for a guided first session, then run `claude --version` to verify.
2. **This repo cloned** — `git clone https://github.com/klesajos/dxc-ws`
3. **A terminal opened in the repo folder** — all examples assume your
   working directory is the repo root (the folder containing `CMakeLists.txt`).

## Quick demo script (5 minutes)

Open Claude Code in the repo (`cd dxc-ws && claude`) and try:

1. **Skill** — ask: *"Add a test for sliding the line {4, 4, 8, 0}"*.
   Watch Claude load the `board-tests` skill before writing the test.
   You can also invoke it yourself by typing `/board-tests`.
2. **Hook** — ask Claude to edit any `.cpp` file. Right after the edit,
   the hook runs `clang-format` on the file — the code is always formatted,
   even if Claude wrote it messy.
3. **MCP** — type `/mcp` to see the two project servers, then ask:
   *"Use deepwiki to look up how Catch2 generators work."*
4. **Plugin** — type `/2048-dev:build-test` to configure, build, and test
   the whole project with one command.
5. **Subagent** — make a trivial edit to any `.cpp`, then ask:
   *"Review my uncommitted changes."* The read-only `cpp-reviewer` agent
   reports findings with `file:line` and modifies nothing.
6. **Workflow** — run `/test-coverage-audit` to fan out a read-only coverage
   audit over `src/` and print a prioritized gap report (re-run it: same shape).

## Platform support at a glance

Claude Code runs in several places, and **not everything works everywhere**.
Each guide has a detailed "Where it works" section; the summary:

| Project-scoped mechanism | CLI (terminal) | Desktop app — Code tab | Cowork |
|--------------------------|:--------------:|:----------------------:|:------:|
| Skills (`.claude/skills/`) | ✅ | ✅ | ❌ — package as a plugin instead |
| Hooks (`.claude/settings.json`) | ✅ | ✅ | ❌ — no local hooks in the sandbox |
| MCP (`.mcp.json`) | ✅ | ✅ | ❌ — use claude.ai **Connectors** instead |
| Plugin (in-repo marketplace) | ✅ | ✅ | ⚠️ — content works, but install via Cowork's plugin management, not project settings |
| Subagents (`.claude/agents/`) | ✅ | ✅ | ❌ — bundle the agent in a plugin instead |
| Workflows (`.claude/workflows/`) | ✅ | ✅* | ❌* — local orchestration; *verify per build |

Why: the CLI and the Desktop **Code tab** run the same engine and share all
project configuration — the Desktop just adds a one-time project trust
dialog. **Cowork** runs tasks in its own sandboxed VM and does not load
project-scoped `.claude/` config; it has its own equivalents (Connectors
for MCP, its own plugin install flow).

Reference: the [Desktop quickstart](https://code.claude.com/docs/en/desktop-quickstart)
states the Code tab "runs the same engine as the CLI ... they share configuration
(CLAUDE.md files, MCP servers, hooks, skills, and settings)." Each numbered guide
below has its own "Where it works" section linking the relevant official docs.

## Which mechanism should I use when?

A common beginner question. Rule of thumb:

- **Skill** — when you want Claude to *know* something (conventions, how-tos).
  Claude decides when to apply it. Think: documentation for the AI.
- **Hook** — when something must happen *every single time*, no exceptions
  (formatting, linting, blocking dangerous commands). The model has no say.
- **MCP** — when Claude needs a *capability* it doesn't have (query a
  database, search docs, control a browser).
- **Plugin** — when you want to *share* any of the above across multiple
  repos or with your whole team, as one versioned package.
- **Subagent** — when you want to hand *one* self-contained task to an
  isolated context with its own tools and persona (a reviewer that can't
  write, a tester, a codebase explorer).
- **Workflow** — when you want *repeatable, multi-stage* orchestration of
  *several* agents in code (fan out over files, gate one stage on another,
  reduce many results into one report).
