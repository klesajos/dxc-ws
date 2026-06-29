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
| `claude --permission-mode plan` | Start in a permission mode (`plan` / `acceptEdits` / `auto` / `dontAsk` / `bypassPermissions`) |
| `claude -p "..." --allowedTools "Read,Edit,Bash(git diff *)"` | Allowlist the tools an unattended / CI run may use |
| `claude -p "..." --output-format json` | Headless output as `json` or `stream-json` for scripts |

## Background sessions (`claude agents`)

A **background session** is a full conversation that keeps running detached (owned by
a supervisor process) — dispatch work, walk away, check back later.

| Command | What it does |
|---------|--------------|
| `claude --bg "..."` | Start a background session from the shell (long form `--background`) |
| `/bg <prompt>` | Background the current work from inside a session (also `/background`) |
| `claude agents` | Open the agent view — monitor / dispatch sessions (`--json` to script it) |
| `claude attach <id>` | Attach to a running background session |
| `claude logs <id>` | Print a background session's output |
| `claude stop <id>` | Stop a session (alias `claude kill`) |
| `claude rm <id>` | Remove a finished session from the list |

**Headless (`-p`) vs. background (`--bg`) — when to use which:**

| | `claude -p "..."` (headless) | `claude --bg "..."` (background) |
|---|---|---|
| Runs | Foreground — prints, then exits | Detached — persists under a supervisor |
| Tied to your shell | Yes (close it → gone) | No (keeps running) |
| In `claude agents`? | No | Yes (attach / logs / stop) |
| Reach for it when | Scripting, CI, a one-off you pipe or parse | A long task you dispatch and revisit while you keep working |

Not to be confused with `/agents` (manage **subagents** in the current session) or
`claude --agent <name>` (run the whole session *as* a named subagent).

## Slash commands

Type `/` to autocomplete. Grouped by what they're for:

**Session & context**

| Command | What it does |
|---------|--------------|
| `/clear` | Wipe the conversation — clean slate for a new task |
| `/compact [instructions]` | Summarise the conversation to reclaim context; optional focus, e.g. `/compact keep the auth work` |
| `/context` | Show what's filling the context window right now |
| `/btw <question>` | Quick side question — answered *without* adding to the conversation history |
| `/rewind` | Jump back to an earlier checkpoint (code and/or conversation) — also `Esc Esc` |
| `/resume` | Switch to another past session without leaving |
| `/rename <name>` | Rename the current session (easier to find later with `--resume`) |
| `/export` | Save the whole conversation to a Markdown file |

**Configuration**

| Command | What it does |
|---------|--------------|
| `/config` | Open the settings menu (model, theme, permissions) |
| `/model` | Switch model mid-session |
| `/permissions` | View / grant standing tool permissions |
| `/sandbox` | Toggle filesystem / network isolation for Bash (or set `"sandbox.enabled": true`) |
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
| `@agent-<name>` | **Delegate** — hand the task to a specific subagent (e.g. `@agent-cpp-reviewer`) |
| `#` | **Add to memory** — save the line into a `CLAUDE.md` for next time |
| `\` + Enter | **Multiline** — continue the prompt on a new line without sending |

- **Paste an image:** `Ctrl+V` (or drag-and-drop) drops a screenshot / mock-up into
  the prompt — great for UI work. On macOS use `Ctrl+V`, *not* `Cmd+V`.
- **Pipe a file:** `cat error.log | claude -p "explain this"` feeds stdin into a
  headless run (needs `-p`).

## Rewind & checkpoints

Every prompt is a **checkpoint**. Open the menu with `/rewind` or `Esc Esc`:

| Restore | Effect |
|---------|--------|
| **Code + conversation** | Roll both back to that prompt |
| **Conversation only** | Rewind the chat, keep the current code |
| **Code only** | Revert file edits, keep the chat |
| **Summarize from / up to here** | Compress the conversation after / before that point |

- Checkpoints persist across sessions and are kept ~30 days (`cleanupPeriodDays`).
- **Not captured:** files changed via Bash (`rm` / `mv` / `cp`) or edited outside
  Claude Code. **Not a Git replacement** — keep committing at milestones.

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
| **bypassPermissions** | Do anything without asking ("YOLO" — sandbox only). Not in the `Shift+Tab` cycle — enable explicitly with `--dangerously-skip-permissions` |

Two more modes exist beyond the everyday `Shift+Tab` cycle — **auto** (act with
background safety checks) and **dontAsk** (only pre-approved tools); see the
[permission-modes docs](https://code.claude.com/docs/en/permission-modes).

## Models & effort

| Model | Best for |
|-------|----------|
| `opus` | Hardest reasoning — architecture, tricky bugs |
| `sonnet` | Daily driver — features, refactors, tests |
| `haiku` | Fast & cheap — boilerplate, quick checks, parallel subagents |
| `opusplan` | Opus to plan, then auto-switches to Sonnet to implement |

**Effort** controls how deeply Claude reasons — higher = better answers, slower,
more tokens. The ladder, low → high: `low` · `medium` · `high` · `xhigh` · `max`.

| Set effort | How |
|------------|-----|
| In session | `/effort` (slider) · `/effort high` (jump to a level) · `/effort auto` (back to the model default) |
| At launch | `claude --effort xhigh "..."` — this session only |
| Persisted | `"effortLevel": "high"` in `settings.json`, or env `CLAUDE_CODE_EFFORT_LEVEL` |
| Per agent / skill | an `effort:` field in the frontmatter |

`max` is session-only; if a model lacks a tier, Claude falls back to its highest
supported one. **One-shot:** drop `ultrathink` anywhere in a prompt to push that
single turn harder *without* changing the session effort — only `ultrathink` is a
real keyword (`think` / `think hard` are just ordinary words).

## Context economics (why `/compact` matters)

- Claude re-reads the **whole** conversation each turn, so cost grows with
  history. `/compact` summarises to stay lean; `/clear` resets for a new task.
- **Prompt caching** gives a large discount on the stable prefix (your
  `CLAUDE.md`, project structure) — it's automatic; keep that prefix stable.
- **Be mindful of extensions:** every active MCP server, skill and subagent
  consumes context. Enable what the task needs, not everything.

## What's legacy in 2026 (the convergence)

Custom slash commands have been **merged into Skills** (Example 1). The old
command files keep working, but new work should use Skills:

| Was its own thing | Now | Status |
|-------------------|-----|--------|
| Custom slash commands (`.claude/commands/*.md`) | **Skills** (`.claude/skills/<name>/SKILL.md`) | Merged — both still create `/name`; legacy files keep working |

(Output styles are a *separate*, still-current feature — they modify the system
prompt, skills load task instructions — see the
[output-styles docs](https://code.claude.com/docs/en/output-styles); only the
standalone `/output-style` command was deprecated, in favour of `/config`.)

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
