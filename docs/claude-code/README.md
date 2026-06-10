> 🌍 Read this in: **English** | [Česky](README.cs.md)

# Claude Code extension examples

This repo doubles as a showcase of the four **project-scoped** ways to extend
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

Every guide exists in two languages: `xx-name.md` is English,
`xx-name.cs.md` is Czech. They have the same content.

## Before you start

You need:

1. **Claude Code installed** — see [code.claude.com](https://code.claude.com)
   for install instructions, then run `claude` in a terminal to verify.
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

## Platform support at a glance

Claude Code runs in several places, and **not everything works everywhere**.
Each guide has a detailed "Where it works" section; the summary:

| Project-scoped mechanism | CLI (terminal) | Desktop app — Code tab | Cowork |
|--------------------------|:--------------:|:----------------------:|:------:|
| Skills (`.claude/skills/`) | ✅ | ✅ | ❌ — package as a plugin instead |
| Hooks (`.claude/settings.json`) | ✅ | ✅ | ❌ — no local hooks in the sandbox |
| MCP (`.mcp.json`) | ✅ | ✅ | ❌ — use claude.ai **Connectors** instead |
| Plugin (in-repo marketplace) | ✅ | ✅ | ⚠️ — content works, but install via Cowork's plugin management, not project settings |

Why: the CLI and the Desktop **Code tab** run the same engine and share all
project configuration — the Desktop just adds a one-time project trust
dialog. **Cowork** runs tasks in its own sandboxed VM and does not load
project-scoped `.claude/` config; it has its own equivalents (Connectors
for MCP, its own plugin install flow).

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
