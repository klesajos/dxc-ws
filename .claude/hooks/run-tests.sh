#!/usr/bin/env bash
# Stop hook: when Claude finishes a turn, run the test suite once and print a
# one-line pass/fail summary. Advisory and non-blocking by design — it never
# stops Claude, it just surfaces whether the tree is still green.
set -euo pipefail

# Drain the Stop-event JSON on stdin. We don't need any of its fields, but a
# hook must read stdin so Claude Code isn't left writing to a closed pipe.
cat >/dev/null

project_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}"
build_dir="$project_dir/build"

# Stay silent until the project has been configured at least once, so the hook
# never nags before there is anything to test.
if [ ! -d "$build_dir" ] || ! command -v ctest >/dev/null 2>&1; then
    exit 0
fi

# Run quietly and keep only ctest's summary line, e.g.
# "100% tests passed, 0 tests failed out of 13".
summary=$(ctest --test-dir "$build_dir" 2>/dev/null | grep -E 'tests passed' | tail -1 || true)

# build/ exists but nothing is built/registered yet — stay silent.
if [ -z "$summary" ]; then
    exit 0
fi

case "$summary" in
    *"0 tests failed"*) echo "run-tests hook: ✓ $summary" ;;
    *) echo "run-tests hook: ✗ $summary (run 'ctest --test-dir build' for details)" ;;
esac

exit 0
