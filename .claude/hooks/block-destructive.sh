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
