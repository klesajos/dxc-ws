# Claude Code extension examples

This repo doubles as a showcase of the four **project-scoped** ways to extend
Claude Code. Each example is independent, minimal, and tied to the 2048
codebase so it can be demoed live. Everything is checked into the repo —
anyone who clones it gets the full setup automatically.

| # | Example | Mechanism | Where it lives |
|---|---------|-----------|----------------|
| 1 | [Skills](01-skills.md) | Reusable instructions Claude loads on demand | `.claude/skills/board-tests/SKILL.md` |
| 2 | [Hooks](02-hooks.md) | Shell commands triggered by Claude Code events | `.claude/settings.json` + `.claude/hooks/format-cpp.sh` |
| 3 | [MCP](03-mcp.md) | External tool servers Claude can call | `.mcp.json` |
| 4 | [Plugins](04-plugins.md) | Bundled commands/skills distributed via a marketplace | `plugins/2048-dev/` + `.claude-plugin/marketplace.json` |

## Quick demo script

1. **Skill** — ask: *"Add a test for sliding the line {4, 4, 8, 0}"* and watch
   the `board-tests` skill load, or invoke it directly with `/board-tests`.
2. **Hook** — ask Claude to edit any `.cpp` file with sloppy formatting; the
   `PostToolUse` hook runs `clang-format` on it immediately after the edit.
3. **MCP** — run `/mcp` to see the project servers, then ask: *"Use deepwiki
   to look up how Catch2 generators work."*
4. **Plugin** — run `/2048-dev:build-test` to configure, build, and test the
   project in one command.
