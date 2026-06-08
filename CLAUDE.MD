# 2048

Terminal version of the game 2048 in modern C++. Used as the base repository
for an AI-assisted coding workshop (adding features, fixing bugs, writing
tests, refactoring).

## Stack

- Language: C++20
- Build system: CMake 3.16+
- Test framework: Catch2 v3 (fetched automatically via CMake FetchContent)

## Directory Structure

- `src/` - source and header files
  - `board.{hpp,cpp}` - game logic: slide, merge, win/game-over detection (no I/O)
  - `game.{hpp,cpp}` - main loop, wires board + input + renderer together
  - `input.{hpp,cpp}` - terminal keyboard input in raw mode
  - `renderer.{hpp,cpp}` - draws the grid and score
  - `main.cpp` - entry point
- `tests/` - Catch2 unit tests (`test_board.cpp`)
- `build/` - CMake build directory (auto-generated)

## Conventions

- C++20 dialect
- Use namespaces (all code lives in namespace `g2048`)
- Code style enforced by `.clang-format` (Google-based, 4-space indent, 88 cols)
- Use the C++ Standard Library; `<cstdint>` for integer types, `std::string` for strings
- Class interfaces in header files with the same base name as the `.cpp` but with `.hpp` extension (e.g. `board.hpp` / `board.cpp`)
- Implementation in `.cpp` files using qualified method definitions, e.g.: `Board::move`
- Keep game rules in `Board`, free of I/O, so they stay unit-testable
- RAII everywhere (e.g. terminal raw mode is restored by a guard's destructor)

## Commands

- configure: `cmake -S . -B build`
- build: `cmake --build build -j`
- run: `./build/2048`
- test: `ctest --test-dir build --output-on-failure`
- clean: `cmake --build build --target clean`

## Build Dependencies

- CMake 3.16+
- GCC 10+ or Clang 12+ (C++20 support)
- Catch2 v3 (downloaded at configure time; requires internet on first build)
- POSIX terminal (`<termios.h>`) - Linux / macOS
