> 🌍 Read this in: **English** | [Česky](cheatsheet.cs.md)

# Claude Code cheat-sheet

The numbered guides teach you how to **extend** Claude Code ([skills](01-skills.md),
[hooks](02-hooks.md), [MCP](03-mcp.md), [plugins](04-plugins.md),
[subagents](05-agents.md), [workflows](06-workflows.md)). This page is the other
half: how to **operate**
it day to day — the flags, slash commands, shortcuts, prefixes and hook events
you reach for constantly but that don't each need a full guide.

> ✅ **Verified against Claude Code 2.1.195 (2026-06-29).** The CLI surface
> changes between releases — check `claude --version` and the
> [official docs](https://code.claude.com/docs/en/cli-reference) if something
> here doesn't match.

## Launch flags (`claude ...`)

| Command | What it does |
|---------|--------------|
| `claude` | Start an interactive session in the current folder |
| `claude "fix the lint errors"` | Start a session with the first prompt already queued |
| `claude -c` / `--continue` | Resume the **most recent** session in this folder |
| `claude -r` / `--resume [id]` | Resume a **specific** session (no id → interactive picker) |
| `claude -p "..."` / `--print` | **Headless**: run the prompt, print the result, exit. For scripts / CI |
| `claude --model opus` | Start on a specific model (`opus`, `sonnet`, `haiku`, `opusplan`) |
| `claude --agent cpp-reviewer` | Run the whole session as a named subagent |
| `claude --add-dir ../lib` | Give the session access to extra folders outside the repo |

## Slash commands

Type `/` to autocomplete. Grouped by what they're for:

**Session & context**

| Command | What it does |
|---------|--------------|
| `/clear` | Wipe the conversation — clean slate for a new task |
| `/compact` | Summarise the conversation so far to reclaim context (keeps the gist) |
| `/context` | Show what's filling the context window right now |
| `/rewind` | Jump back to an earlier checkpoint (code and/or conversation) — also `Esc Esc` |
| `/resume` | Switch to another past session without leaving |
| `/export` | Save the whole conversation to a Markdown file |

**Configuration**

| Command | What it does |
|---------|--------------|
| `/config` | Open the settings menu (model, theme, permissions) |
| `/model` | Switch model mid-session |
| `/permissions` | View / grant standing tool permissions |
| `/memory` | Edit persistent memory (`CLAUDE.md` files) |
| `/init` | Generate a starter `CLAUDE.md` for this repo |
| `/status`, `/statusline` | Session health dashboard / customise the bottom info bar |
| `/doctor` | Diagnose a broken install (auth, network, dependencies) |

**Extensions** (the six mechanisms the guides cover)

| Command | What it does |
|---------|--------------|
| `/skill-name` | Run a skill, e.g. `/board-tests` (Example 1) |
| `/2048-dev:build-test` | Run a plugin's bundled skill (Example 4) |
| `/agents` | Create / manage subagents |
| `/hooks` | View / manage hooks |
| `/mcp` | View MCP servers and authorise them |
| `/plugin` | Install / manage plugins and marketplaces |

**Info**

| Command | What it does |
|---------|--------------|
| `/help` | List all commands, shortcuts and features |
| `/usage` | Remaining capacity on a Pro/Max plan |
| `/cost` | USD / token spend for the current session (API billing) |

## Keyboard shortcuts

| Key | What it does |
|-----|--------------|
| `Ctrl+C` | Interrupt the current action (kill switch) |
| `Ctrl+R` | Reverse-search your prompt/command history |
| `Esc` | Cancel the current input / close a menu |
| `Esc Esc` | Open **Rewind** — restore an earlier checkpoint |
| `Shift+Tab` | Cycle permission modes (default → accept-edits → plan) |
| `Ctrl+G` | Open the plan file in your editor (in Plan mode) |

## Input prefixes

| Prefix | What it does |
|--------|--------------|
| `!` | **Bash mode** — run a shell command directly, no model, no tokens (e.g. `!git status`) |
| `@` | **File mention** — pull a specific file/folder into context (autocompletes) |
| `#` | **Add to memory** — save the line into a `CLAUDE.md` for next time |
| `\` + Enter | **Multiline** — continue the prompt on a new line without sending |

## Hook events

Where a hook can attach in the session lifecycle (see [Example 2](02-hooks.md)):

| Event | Fires | Typical use |
|-------|-------|-------------|
| `SessionStart` | Session begins | Load context, environment checks |
| `UserPromptSubmit` | You submit a prompt | Inject context, validate/rewrite the prompt |
| `PreToolUse` | Before a tool runs | **Block** dangerous actions (e.g. `git push --force`) |
| `PostToolUse` | After a tool succeeds | Format / lint — *this repo's `format-cpp` hook* |
| `Stop` | Claude finishes its turn | Verify tests — *this repo's `run-tests` hook* |
| `SessionEnd` | Session ends | Cleanup, archiving, reporting |

## Permission modes (cycle with `Shift+Tab`)

| Mode | Claude may… |
|------|-------------|
| **default** | Ask before every edit and command |
| **acceptEdits** | Edit files freely, still ask before shell commands |
| **plan** | Read-only — research and draft a plan, change nothing |
| **bypassPermissions** | Do anything without asking ("YOLO" — sandbox only) |

## Models & effort

| Model | Best for |
|-------|----------|
| `opus` | Hardest reasoning — architecture, tricky bugs |
| `sonnet` | Daily driver — features, refactors, tests |
| `haiku` | Fast & cheap — boilerplate, quick checks, parallel sub-agents |
| `opusplan` | Opus to plan, then auto-switches to Sonnet to implement |

Tune reasoning depth with **effort** (`low` / `medium` / `high`, some models add
higher tiers) via `/model` or settings. Higher effort = deeper thinking, more
tokens, slower.

## Context economics (why `/compact` matters)

- Claude re-reads the **whole** conversation each turn, so cost grows with
  history. `/compact` summarises to stay lean; `/clear` resets for a new task.
- **Prompt caching** gives a large discount on the stable prefix (your
  `CLAUDE.md`, project structure) — it's automatic; keep that prefix stable.
- **Be mindful of extensions:** every active MCP server, skill and subagent
  consumes context. Enable what the task needs, not everything.

## What's legacy in 2026 (the convergence)

Anthropic is collapsing several overlapping authoring surfaces into **Skills**
(Example 1). Old files keep working, but new work should use Skills:

| Was its own thing | Now | Status |
|-------------------|-----|--------|
| Custom slash commands (`.claude/commands/*.md`) | **Skills** (`.claude/skills/<name>/SKILL.md`) | Merged — both still create `/name`; legacy files keep working |
| Output styles | **Skills** | Moving to Skills |

A skill is a strict superset of an old command: same `/name` invocation, **plus**
optional autonomous loading, a supporting-files folder, and invocation control.
Want a skill to behave like the old "manual only" command? Add
`disable-model-invocation: true` to its frontmatter.

Source: [official Skills docs](https://code.claude.com/docs/en/skills) — *"Custom
commands have been merged into skills."*

---

**Keep this open while you work.** It's reference, not a tutorial — for the *how*
and *why* of each extension mechanism, see the numbered guides in the
[index](README.md).
