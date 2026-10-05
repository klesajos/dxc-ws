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
