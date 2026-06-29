---
name: build-test
description: Configure, build, and run the full test suite, then summarize results
allowed-tools: Bash(cmake:*), Bash(ctest:*)
disable-model-invocation: true
---

# Build and test

Build the project and run the tests:

1. Configure if needed: `cmake -S . -B build`
2. Build: `cmake --build build -j`
3. Test: `ctest --test-dir build --output-on-failure`

Then report:
- Build: OK, or the first compiler error with file:line.
- Tests: passed/failed counts. For each failure, name the TEST_CASE and show
  the failing REQUIRE with actual vs expected values.
- Do not fix anything — this skill only reports status.
