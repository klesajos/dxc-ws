#!/usr/bin/env bash
# PostToolUse hook: clang-format any C++ file Claude just edited or wrote.
# Receives the tool call as JSON on stdin; non-C++ files are ignored.
set -euo pipefail

input=$(cat)

if command -v jq >/dev/null 2>&1; then
    file_path=$(printf '%s' "$input" | jq -r '.tool_input.file_path // empty')
else
    file_path=$(printf '%s' "$input" | python3 -c \
        'import json,sys; print(json.load(sys.stdin).get("tool_input", {}).get("file_path", ""))')
fi

# clang-format may live behind xcrun on macOS instead of being on PATH.
if command -v clang-format >/dev/null 2>&1; then
    formatter="clang-format"
elif command -v xcrun >/dev/null 2>&1 && xcrun --find clang-format >/dev/null 2>&1; then
    formatter="xcrun clang-format"
else
    exit 0
fi

case "$file_path" in
    *.cpp|*.hpp)
        if [ -f "$file_path" ]; then
            $formatter -i "$file_path"
            echo "format-cpp hook: formatted ${file_path##*/}"
        fi
        ;;
esac

exit 0
