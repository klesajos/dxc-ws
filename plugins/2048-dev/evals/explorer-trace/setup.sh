#!/usr/bin/env bash
# Runs in the empty eval workspace before Claude starts (only with --scaffold).
# Copies the trimmed 2048 call chain into the workspace as src/.
set -euo pipefail
mkdir -p src
cp "$(dirname "$0")"/fixture/*.cpp src/
