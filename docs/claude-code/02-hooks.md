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
turn — and explains the three hook **types**.

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

The script (`.claude/hooks/run-tests.sh`) is deliberately **advisory**: it runs
`ctest`, prints one line, and always `exit 0` — so it never interrupts you.

```bash
cat >/dev/null                         # drain the Stop-event JSON we don't need
build_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}/build"
[ -d "$build_dir" ] && command -v ctest >/dev/null 2>&1 || exit 0   # quiet until built
summary=$(ctest --test-dir "$build_dir" 2>/dev/null | grep -E 'tests passed' | tail -1 || true)
[ -z "$summary" ] && exit 0
case "$summary" in
    *"0 tests failed"*) echo "run-tests hook: ✓ $summary" ;;
    *) echo "run-tests hook: ✗ $summary (run 'ctest --test-dir build' for details)" ;;
esac
```

- It stays **silent until you've configured the build** (`build/` exists), so
  it never nags on a fresh clone.
- It reports `✓` when everything passes and `✗` otherwise — a passive safety
  net that tells you the moment a change breaks a test.

**Want it to *block* instead of inform?** A `Stop` hook that exits non-zero (or
prints `{"decision": "block", "reason": "..."}` on stdout) tells Claude it is
**not** done — that's how you enforce "tests must pass before you stop". We keep
ours advisory so a live workshop session never gets stuck in a fix-tests loop;
flip it to blocking when you want a hard gate.

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
