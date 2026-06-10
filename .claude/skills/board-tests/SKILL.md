---
name: board-tests
description: Use when writing or extending Catch2 unit tests for the 2048 board logic (slideLineLeft, Board::move, win/game-over detection). Covers project test conventions and how to run the suite.
---

# Writing board tests

All game rules live in `src/board.{hpp,cpp}` and are free of I/O — that is what
makes them unit-testable. Tests go in `tests/test_board.cpp` (Catch2 v3).

## Conventions

- One `TEST_CASE` per behaviour, with a plain-English name describing the
  scenario, e.g. `TEST_CASE("two separate pairs merge into two tiles")`.
- Arrange-Act-Assert: build the state, perform one action, then `REQUIRE`.
- Prefer testing the free function `slideLineLeft()` for merge rules — it is
  the heart of the game and takes a plain `std::array<int, kSize>&`.
- For whole-board behaviour, use the deterministic constructor
  `Board(Grid grid, int score = 0)` — it spawns no random tiles.
- Never test through `Game`, `Input`, or `Renderer`; rules belong to `Board`.
- Reuse the local `slid()` helper in `test_board.cpp` when the score gained
  is irrelevant; call `slideLineLeft()` directly when asserting the score.

## Key API (from `src/board.hpp`)

```cpp
int slideLineLeft(std::array<int, kSize>& line);  // returns score gained
Board(Grid grid, int score = 0);                  // deterministic, for tests
bool Board::move(Direction dir);                  // true if board changed
bool Board::isGameOver() const;
bool Board::hasWon() const;                       // any tile == kWinValue
```

## Running the tests

```bash
cmake -S . -B build          # once
cmake --build build -j
ctest --test-dir build --output-on-failure
```

After adding a test, always build and run the suite before declaring it done.
