> 🌍 Read this in: **English** | [Česky](02-hooks.cs.md)

# Example 2: Project-scoped hook

## What is a hook?

A **hook** is a shell command that Claude Code runs **automatically** when a
certain event happens — for example "after Claude edits a file" or "before
Claude runs a terminal command".

The key difference from a skill: a skill is *advice* the model may follow;
a hook is *enforcement* that runs outside the model, **every single time**.
Use hooks for things that must never be skipped: formatting, linting,
blocking dangerous commands.

## What this example does

Every time Claude edits or creates a file, our hook checks whether it's a
C++ file (`.cpp` / `.hpp`) — and if so, runs `clang-format` on it. Result:
Claude's code always lands formatted according to the project's
`.clang-format` rules, even if the model wrote it messy.

Two files are involved:

```
.claude/settings.json        ← registers WHEN to run the hook
.claude/hooks/format-cpp.sh  ← the script that says WHAT to do
```

This guide walks through that format hook in full, then adds a **second hook**
on a different event — a `Stop` hook that runs the tests when Claude finishes a
turn — then two **guard-rail and context hooks** (`PreToolUse`,
`UserPromptSubmit`), and explains the three hook **types**.

## Part 1: the registration (`.claude/settings.json`), line by line

```json
{
  "hooks": {
    "PostToolUse": [
      {
        "matcher": "Edit|Write",
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/format-cpp.sh"
          }
        ]
      }
    ]
  }
}
```

- `"hooks"` — the top-level section of settings where all hooks live.
- `"PostToolUse"` — the **event**: fire *after* Claude successfully uses a
  tool. Other useful events: `PreToolUse` (before a tool runs — can block
  it), `SessionStart`, `UserPromptSubmit`, `Stop` (when Claude finishes).
- `"matcher": "Edit|Write"` — a filter on the **tool name**. The `|` means
  "or", like in regular expressions: run only when the tool was `Edit` or
  `Write` (the two tools Claude uses to change files). Without a matcher the
  hook would also fire after every `Read`, `Bash`, etc.
- `"type": "command"` — this hook runs a shell command (other types exist,
  e.g. calling an HTTP endpoint).
- `"command": "${CLAUDE_PROJECT_DIR}/..."` — the script to run.
  `${CLAUDE_PROJECT_DIR}` is a variable Claude Code replaces with the
  absolute path of the repo root — so the hook works no matter which
  subdirectory Claude is currently in.

## Part 2: the script (`.claude/hooks/format-cpp.sh`), line by line

```bash
#!/usr/bin/env bash
set -euo pipefail

input=$(cat)
```

- `#!/usr/bin/env bash` — the "shebang": tells the OS to run this file
  with bash.
- `set -euo pipefail` — safety switches: stop on any error (`-e`), treat
  unset variables as errors (`-u`), fail a pipeline if any step fails
  (`pipefail`). Standard practice for every bash script.
- `input=$(cat)` — **this is how hooks receive data.** Claude Code sends
  the details of the tool call as JSON on **stdin** (standard input).
  `cat` reads all of it; we store it in the variable `input`.

```bash
if command -v jq >/dev/null 2>&1; then
    file_path=$(printf '%s' "$input" | jq -r '.tool_input.file_path // empty')
else
    file_path=$(printf '%s' "$input" | python3 -c \
        'import json,sys; print(json.load(sys.stdin).get("tool_input", {}).get("file_path", ""))')
fi
```

- `command -v jq` — checks whether the JSON tool `jq` is installed
  (`>/dev/null 2>&1` just hides the output of the check).
- `jq -r '.tool_input.file_path // empty'` — extracts the edited file's
  path from the JSON. The input looks like
  `{"tool_name": "Edit", "tool_input": {"file_path": "/path/to/board.cpp", ...}}`,
  so `.tool_input.file_path` navigates to the path. `// empty` means
  "if missing, output nothing instead of the word null".
- The `else` branch does exactly the same with Python — a fallback for
  machines without `jq`.

```bash
if command -v clang-format >/dev/null 2>&1; then
    formatter="clang-format"
elif command -v xcrun >/dev/null 2>&1 && xcrun --find clang-format >/dev/null 2>&1; then
    formatter="xcrun clang-format"
else
    exit 0
fi
```

- Finds a usable formatter. On macOS, `clang-format` often isn't on PATH
  but ships with Xcode command-line tools, reachable via `xcrun`.
- `exit 0` — if there's no formatter at all, exit **successfully** and do
  nothing. A non-zero exit code would surface as an error to Claude;
  "formatter not installed" shouldn't break the session.

```bash
case "$file_path" in
    *.cpp|*.hpp)
        if [ -f "$file_path" ]; then
            $formatter -i "$file_path"
            echo "format-cpp hook: formatted ${file_path##*/}"
        fi
        ;;
esac

exit 0
```

- `case ... in *.cpp|*.hpp)` — pattern match on the file extension. Only
  C++ sources and headers proceed; a `.md` or `.json` file falls through
  and the script just exits.
- `[ -f "$file_path" ]` — "does the file exist?" (it might have been
  deleted in the meantime).
- `$formatter -i "$file_path"` — the actual work: `-i` means "in place",
  i.e. rewrite the file with formatted content.
- `echo ...` — whatever a hook prints is shown in the Claude Code
  transcript, so you can see the hook did its job.
  (`${file_path##*/}` strips the directory part, leaving just the filename.)

## Create your own hook, step by step

1. **Write a script** in `.claude/hooks/`, e.g. `my-hook.sh`. Start from
   the skeleton above: read stdin, extract what you need, act, `exit 0`.
2. **Make it executable** — this is the step everyone forgets:
   ```bash
   chmod +x .claude/hooks/my-hook.sh
   ```
3. **Register it** in `.claude/settings.json` under the right event +
   matcher (see Part 1).
4. **Test it manually first** — don't debug inside Claude. Fake the stdin
   JSON yourself:
   ```bash
   echo '{"tool_input":{"file_path":"src/board.cpp"}}' | .claude/hooks/my-hook.sh
   ```
5. **Start a new Claude Code session** and trigger the event for real.
6. **Commit both files.**

## Try the demo

Ask Claude to add a method to `src/board.cpp` and not worry about
formatting. After the edit, run `git diff` — the code is already formatted,
and the line `format-cpp hook: formatted board.cpp` appears in the
transcript.

## A second hook: gate on tests (the `Stop` event)

The format hook reacts to a **tool** (`Edit`/`Write`). Hooks can also react to
the **session lifecycle**. This repo ships a second hook on the `Stop` event —
which fires once, when Claude finishes its turn — to run the test suite and
report whether the tree is still green.

Its registration sits next to the first hook in `.claude/settings.json`. Note
there is **no `matcher`**: `Stop` isn't about a tool, so there's nothing to
filter on.

```json
"Stop": [
  {
    "hooks": [
      { "type": "command",
        "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/run-tests.sh" }
    ]
  }
]
```

The script (`.claude/hooks/run-tests.sh`) is deliberately **advisory**: it
drains the Stop-event JSON, runs `ctest`, prints one line, and always `exit 0` —
so it never interrupts you. Its executable lines, verbatim from the file:

```bash
cat >/dev/null
project_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}"
build_dir="$project_dir/build"
if [ ! -d "$build_dir" ] || ! command -v ctest >/dev/null 2>&1; then
    exit 0
fi
summary=$(ctest --test-dir "$build_dir" 2>/dev/null | grep -E 'tests passed' | tail -1 || true)
if [ -z "$summary" ]; then
    exit 0
fi
case "$summary" in
    "100% tests passed"*) echo "run-tests hook: ✓ $summary" ;;
    *) echo "run-tests hook: ✗ $summary (run 'ctest --test-dir build' for details)" ;;
esac
```

- **The guard** `if [ ! -d "$build_dir" ] || ! command -v ctest …; then exit 0; fi`
  bails out if there's no `build/` directory **or** no `ctest` on PATH — so the
  hook stays silent on a fresh, unbuilt clone instead of erroring.
- **The summary line** `ctest … | grep -E 'tests passed' | tail -1` keeps only
  ctest's one-line tally (e.g. `100% tests passed out of 13` on ctest 4.x);
  `|| true` keeps a failing run from aborting under `set -e`, and the following
  `[ -z "$summary" ]` exits quietly if the build registers no tests yet.
- **The verdict** — the `case` prints `run-tests hook: ✓ $summary` when the tally
  starts with `100% tests passed`, otherwise `run-tests hook: ✗ $summary` with a hint
  to rerun `ctest`.

**Want it to *block* instead of inform?** A `Stop` hook that exits non-zero (or
prints `{"decision": "block", "reason": "..."}` on stdout) tells Claude it is
**not** done — that's how you enforce "tests must pass before you stop". We keep
ours advisory so a live workshop session never gets stuck in a fix-tests loop;
flip it to blocking when you want a hard gate.

## Try the second hook

Build the project once (`cmake --build build`), then ask Claude anything. When
the turn finishes, the `Stop` hook runs the suite and an advisory line appears in
the transcript:

```
run-tests hook: ✓ 100% tests passed out of 13
```

Before the first build the hook stays silent — that's the guard doing its job.

## Two more hooks: a guard rail and live git context

The repo ships two more `command` hooks. Together with the first two they cover
the four events you'll use most: `UserPromptSubmit` → `PreToolUse` →
`PostToolUse` → `Stop`.

Their registrations, verbatim from `.claude/settings.json`:

```json
    "UserPromptSubmit": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/git-context.sh"
          }
        ]
      }
    ],
    "PreToolUse": [
      {
        "matcher": "Bash",
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/block-destructive.sh"
          }
        ]
      }
    ],
```

### Guard rail: `block-destructive.sh` (`PreToolUse`)

Fires **before** every Bash command and refuses the ones that destroy work git
can't give back: recursive force-deletes (except `rm -rf build`),
`git reset --hard`, force-push (`--force-with-lease` is allowed),
`git clean -f` and `git checkout -- .` / `git restore .`.

```bash
#!/usr/bin/env bash
# PreToolUse hook (matcher: Bash): refuse shell commands that destroy work
# which git can't give back. Claude receives the reason and picks a safer
# route; everything else passes through untouched.
set -euo pipefail

input=$(cat)

if command -v jq >/dev/null 2>&1; then
    command=$(printf '%s' "$input" | jq -r '.tool_input.command // empty')
else
    command=$(printf '%s' "$input" | python3 -c \
        'import json,sys; print(json.load(sys.stdin).get("tool_input", {}).get("command", ""))')
fi

deny() {
    printf '{"hookSpecificOutput":{"hookEventName":"PreToolUse","permissionDecision":"deny","permissionDecisionReason":"%s"}}\n' "$1"
    exit 0
}

# Wiping the build directory is routine; any other recursive force-delete is not.
# Flags are checked separately so `rm -rf`, `rm -fr` and `rm -r -f` all match.
if printf '%s' "$command" | grep -Eq '(^|[;&|[:space:]])rm[[:space:]]' \
    && printf '%s' "$command" | grep -Eq '[[:space:]](-[[:alpha:]]*[rR]|--recursive)' \
    && printf '%s' "$command" | grep -Eq '[[:space:]](-[[:alpha:]]*f|--force)' \
    && ! printf '%s' "$command" | grep -Eq '^rm -rf (\./)?build/?$'; then
    deny "block-destructive hook: recursive force-delete is blocked. Only 'rm -rf build' is allowed; delete specific files instead."
fi

if printf '%s' "$command" | grep -Eq 'git[[:space:]]+reset[[:space:]].*--hard'; then
    deny "block-destructive hook: git reset --hard discards uncommitted work. Use git stash, or git reset --soft."
fi

# --force-with-lease is the safe variant, so only bare -f / --force is blocked.
if printf '%s' "$command" | grep -Eq 'git[[:space:]]+push([[:space:]].*)?[[:space:]](-f|--force)([[:space:]]|$)'; then
    deny "block-destructive hook: force-push is blocked. Use git push --force-with-lease, or ask the user."
fi

if printf '%s' "$command" | grep -Eq 'git[[:space:]]+clean[[:space:]]+-[[:alpha:]]*f'; then
    deny "block-destructive hook: git clean -f deletes untracked files for good. Run git clean -n first and show the user the list."
fi

if printf '%s' "$command" | grep -Eq 'git[[:space:]]+(checkout[[:space:]]+(--[[:space:]]+)?|restore[[:space:]]+)\.([[:space:]]|$)'; then
    deny "block-destructive hook: this discards every uncommitted change. Restore individual files, or git stash."
fi

exit 0
```

- **How it blocks:** it prints a JSON object with
  `"permissionDecision": "deny"` inside `hookSpecificOutput` and exits 0. The
  `permissionDecisionReason` goes **to Claude**, so the message tells it what to
  do instead ("use `--force-with-lease`"). Exiting 2 with the reason on stderr
  blocks too.
- **Silence = allow.** No output and exit 0 means "no opinion": the normal
  permission rules and mode decide.
- **It matches text, so expect false positives.** While writing this guide, the
  hook blocked a command that only *contained* the words `git reset --hard`
  inside a heredoc. Pattern hooks err on the side of blocking. That's fine for a
  guard rail, but tell people how to work around it (here: put the text in a
  file).
- **Hook vs. deny rule:** a `Bash(git push --force *)` deny rule in
  `settings.json` ([Example 9](09-permissions-sandbox.md)) is simpler, but it
  can't make exceptions (`rm -rf build` yes, `rm -rf src` no) or explain itself.
  Both match **command text**, so neither is a security boundary:
  `bash -c '…'` gets around both. For a real boundary, use the sandbox.

### Live context: `git-context.sh` (`UserPromptSubmit`)

Fires when you submit a prompt, **before** Claude sees it, and adds one line
such as `Git: on branch main, 2 uncommitted file(s): src/board.cpp, tests/test_board.cpp`.

```bash
#!/usr/bin/env bash
# UserPromptSubmit hook: before Claude sees each prompt, tell it which branch
# it is on and which files are uncommitted. Saves Claude a `git status` call
# and stops it from assuming a clean tree.
set -euo pipefail

# Drain the prompt JSON on stdin; this hook doesn't need any of its fields.
cat >/dev/null

project_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}"
cd "$project_dir"

# Outside a git checkout there's nothing to report.
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || exit 0

branch=$(git branch --show-current 2>/dev/null)
branch=${branch:-"(detached HEAD)"}
changes=$(git status --porcelain 2>/dev/null)

if [ -z "$changes" ]; then
    context="Git: on branch $branch, working tree clean."
else
    count=$(printf '%s\n' "$changes" | wc -l | tr -d ' ')
    # Cap the list so a huge diff can't flood the context window.
    files=$(printf '%s\n' "$changes" | head -10 | awk '{print $NF}' | paste -sd ',' - | sed 's/,/, /g')
    context="Git: on branch $branch, $count uncommitted file(s): $files"
    [ "$count" -gt 10 ] && context="$context, ..."
fi

# json.dumps escapes quotes and backslashes in file names.
CONTEXT="$context" python3 -c '
import json, os
print(json.dumps({"hookSpecificOutput": {
    "hookEventName": "UserPromptSubmit",
    "additionalContext": os.environ["CONTEXT"]}}))'
```

- **How it adds context:** JSON with `hookSpecificOutput.additionalContext`.
  Claude Code wraps the string in a system reminder that Claude reads, but it
  doesn't appear as a chat message. Plain stdout works too; JSON is safer
  because `json.dumps` escapes odd file names.
- **Keep it small:** this runs on **every** prompt, so it costs tokens every
  time. The list is capped at 10 files, and the docs cap any hook's
  `additionalContext` at 10,000 characters.
- **It must be fast:** the prompt waits for the hook. `git status` is
  milliseconds; a network call here would make every prompt feel slow.

## Try the two new hooks

1. Ask Claude: *"Run exactly this command: git push --force nowhere main"*.
   The hook blocks it before git runs. Claude quotes
   `block-destructive hook: force-push is blocked…` and suggests
   `--force-with-lease`. (The remote `nowhere` doesn't exist, so the test is
   harmless even without the hook.)
2. Edit any file, then ask: *"Without running any tool: which branch am I on
   and what's uncommitted?"* Claude answers from the injected context.

Test a hook without Claude by piping a fake event into it:

```bash
echo '{"tool_input":{"command":"git push -f"}}' | .claude/hooks/block-destructive.sh
```

## Hook types: command, prompt, agent

Both hooks above use `"type": "command"` — a shell script. That's the most
common type, but not the only one. A handler can be one of three types, trading
determinism for judgement:

| Type | What runs | Use it for |
|------|-----------|------------|
| `command` | A shell script (our two hooks) | Deterministic, fast checks — format, lint, run tests, block a forbidden command |
| `prompt` | A single-turn call to a small Claude model | A quick judgement call — "is this commit message descriptive?" — returning a yes/no decision |
| `agent` | A multi-turn subagent with tool access (`Read`, `Grep`, …) | Deep verification — "read the diff and confirm no secrets were added" — before proceeding |

A `prompt` hook is registered with the text to evaluate instead of a command:

```json
{ "type": "prompt",
  "prompt": "Does the staged diff add a test for every new public method? Answer yes or no." }
```

Rule of thumb: reach for `command` first (free and instant), `prompt` when the
check needs language understanding, and `agent` only when the check itself has
to explore the codebase. This repo ships `command` hooks; the other two are
worth knowing exist.

## Optional parameters

A hook handler accepts more than `type` and `command`. The most useful
optional fields:

| Field | What it does |
|-------|--------------|
| `timeout` | Seconds before the hook is cancelled (default 600). Set low for fast hooks so a stuck script can't stall the session |
| `statusMessage` | Custom spinner text while the hook runs, e.g. `"Formatting C++..."` |
| `if` | Extra filter using permission-rule syntax, e.g. `"if": "Edit(*.cpp)"` — more precise than `matcher`, which only sees the tool name |
| `once: true` | Run only once per session, then deregister (useful for setup checks) |

And the events: this example uses `PostToolUse`, but hooks can attach to
the whole session lifecycle. The ones worth knowing first:

| Event | Fires |
|-------|-------|
| `PreToolUse` | Before a tool runs — **can block it** (e.g. forbid `git push --force`) |
| `PostToolUse` | After a tool succeeds (our case) |
| `SessionStart` | When a session begins — environment checks, loading context |
| `UserPromptSubmit` | When you submit a prompt — can inject extra context |
| `Stop` | When Claude finishes its turn — e.g. verify tests were actually run |

Full list of events and fields: [official hooks documentation](https://code.claude.com/docs/en/hooks).

## More hook recipes

Ideas worth knowing, not shipped in this repo, either because they're personal
(notifications), need infrastructure (an audit endpoint) or don't fit a C++
game. Every event below exists in Claude Code 2.1.283:

| Recipe | Event | What it does |
|--------|-------|--------------|
| Run only the relevant tests | `PostToolBatch` | After a batch of parallel edits resolves, run the tests for the touched files only, once per batch instead of once per edit |
| Desktop notification | `Notification` | `osascript -e 'display notification "Claude needs you"'` when Claude waits for permission (`permission_prompt`) or input (`idle_prompt`). Belongs in your **personal** `~/.claude/settings.json` |
| Protect files | `PreToolUse` (`Edit\|Write`) | Deny edits to lockfiles, `.git/`, generated code. For plain path blocks, an `Edit(...)` deny rule ([Example 9](09-permissions-sandbox.md)) is simpler |
| Secret scan | `PreToolUse` / `Stop` | Reject edits containing keys, or run `gitleaks` on the diff before Claude finishes |
| Commit gate | `PreToolUse` (`Bash`, `"if": "Bash(git commit *)"`) | Run lint/tests or check the commit-message format before `git commit` |
| AI review gate | `Stop`, `"type": "prompt"` | A small model checks "does the change satisfy the request?" and blocks the stop if not. Costs a model call per turn |
| Load the dev environment | `SessionStart` + `CwdChanged` | Run `direnv` or similar when the session starts or Claude `cd`s |
| Re-inject constraints | `PostCompact` | After context compaction, remind Claude of the active ticket or acceptance criteria |
| Watch config changes | `ConfigChange` | Log or block mid-session edits to settings, rules or skills. Useful in regulated environments |
| Audit trail | `PostToolUse`, `"type": "http"` | POST every tool call to an internal endpoint |
| Redact the screen | `MessageDisplay` | Strip hostnames or customer IDs from what's rendered, without changing the transcript |

## Where it works: CLI, Desktop app, Cowork

| Platform | Works? | Setup |
|----------|--------|-------|
| **Claude Code CLI** (terminal) | ✅ Yes | Nothing extra — hooks in `.claude/settings.json` load at session start |
| **Claude Desktop app — Code tab** | ✅ Yes | Same engine, same config files as the CLI. Confirm the one-time project trust dialog; hooks then run identically |
| **Cowork** (in the Desktop app) | ❌ No | Cowork's sandboxed VM does not execute project-scoped hooks from `.claude/settings.json`. There is no direct equivalent — hooks shipped inside an installed plugin are the closest option |

Note for the workshop: this is the clearest platform difference of the six
examples. Hooks are a *local automation* feature — if your workflow depends
on them (formatting, lint gates), run it in the CLI or the Desktop Code tab,
not in Cowork.

## Troubleshooting

- **Hook never runs** → new session needed after editing `settings.json`;
  also check the script is executable (`ls -l .claude/hooks/`).
- **"Permission denied"** → you skipped `chmod +x`.
- **Hook errors break the flow** → make sure every "nothing to do" path
  ends in `exit 0`, not an error.
