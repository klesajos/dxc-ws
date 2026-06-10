# Example 1: Project-scoped skill

**What it is:** a skill is a Markdown file with instructions that Claude loads
only when relevant — project knowledge that doesn't bloat every prompt the way
a long `CLAUDE.md` would.

**Where it lives:**

```
.claude/skills/board-tests/SKILL.md
```

Because it sits under `.claude/` in the repo, everyone who clones the project
gets it. (Personal skills go in `~/.claude/skills/` instead.)

## Anatomy

```markdown
---
name: board-tests
description: Use when writing or extending Catch2 unit tests for the 2048 board logic...
---

# Writing board tests
...instructions Claude follows when the skill is active...
```

- `description` is the trigger: Claude reads all skill descriptions at session
  start and loads the full body only when the task matches.
- The body holds the actual conventions — test style, key APIs, run commands.

## How it activates

- **Automatically:** ask *"Add a test for sliding the line {4, 4, 8, 0}"* —
  the request matches the description, so Claude loads the skill before
  writing the test.
- **Manually:** type `/board-tests` to invoke it yourself.

## Why this example

`tests/test_board.cpp` has a deliberate `TODO (participants)` for exactly this
line — the skill ensures every attendee's Claude writes the test the same way:
deterministic `Board(Grid)` constructor, AAA structure, `ctest` verification.
