# Example 2: Project-scoped hook

**What it is:** a hook is a shell command Claude Code runs automatically when
an event fires — deterministic automation that doesn't depend on the model
remembering to do something.

**Where it lives:**

```
.claude/settings.json        # hook registration (project-scoped, checked in)
.claude/hooks/format-cpp.sh  # the script it runs
```

## What this one does

After every `Edit` or `Write` tool call (`PostToolUse` event), the hook runs
`clang-format -i` on the touched file — but only if it's a `.cpp`/`.hpp`.
The project's `.clang-format` (Google-based, 4-space indent, 88 cols) is
applied, so Claude's edits always land already formatted.

## How it's wired

```json
{
  "hooks": {
    "PostToolUse": [
      {
        "matcher": "Edit|Write",
        "hooks": [
          { "type": "command", "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/format-cpp.sh" }
        ]
      }
    ]
  }
}
```

- `PostToolUse` fires after a tool succeeds; `matcher` is a regex over tool
  names, so this only runs for file edits — not for `Bash`, `Read`, etc.
- The script receives the tool call as **JSON on stdin** and pulls out
  `tool_input.file_path`; non-C++ paths exit silently with code 0.
- `${CLAUDE_PROJECT_DIR}` resolves to the repo root, so the hook works
  regardless of Claude's current working directory.

## Other useful events

`PreToolUse` (can block a tool call), `SessionStart`, `UserPromptSubmit`,
`Stop` (e.g. force tests to pass before Claude finishes), `Notification`.

## Demo

Ask Claude to add a method to `src/board.cpp` with deliberately bad
formatting — `git diff` afterwards shows clang-format already cleaned it up,
and the hook's output line appears in the transcript.
