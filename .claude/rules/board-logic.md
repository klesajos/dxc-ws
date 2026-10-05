---
paths:
  - "src/board.cpp"
  - "src/board.hpp"
---

# Board logic rules

These rules load only when Claude reads or edits the board files.

- `Board` stays free of I/O: no `<iostream>`, no terminal calls, no
  `std::rand()` seeding inside game rules. Rendering belongs in `renderer.cpp`.
- Keep `slideLineLeft()` a free function so tests can call it directly with a
  `std::array<int, kSize>`.
- Every behaviour change needs a deterministic test that builds the board with
  the `Board(Grid grid, int score)` constructor, never with `spawnRandom()`.
- After any change, run `ctest --test-dir build --output-on-failure` and
  report the result.
