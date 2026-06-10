# 2048 (terminálová verze)

Malá hra 2048 v moderním C++ (C++20). Repo slouží jako základ pro hands-on
část workshopu o AI-asistovaném programování — budeme do něj přidávat funkce,
opravovat chyby, dopisovat testy a refaktorovat.

## Build & spuštění

Potřebuješ kompilátor s C++20 (GCC 10+, Clang 12+) a CMake 3.16+.

```bash
cmake -S . -B build
cmake --build build -j
./build/2048
```

Při prvním buildu si CMake přes `FetchContent` stáhne testovací framework
[Catch2](https://github.com/catchorg/Catch2) — je tedy potřeba internet.

## Testy

```bash
cd build && ctest --output-on-failure
```

## Ovládání

Šipky nebo `WASD` pro pohyb, `q` pro ukončení. Tiles se posouvají daným směrem
a stejné hodnoty se sloučí. Cíl je dosáhnout dlaždice **2048**.

## Struktura

```
src/
  board.{hpp,cpp}      herní logika — posun, slučování, detekce výhry/konce (bez I/O)
  game.{hpp,cpp}       hlavní smyčka, propojuje board + input + renderer
  input.{hpp,cpp}      čtení kláves z terminálu (raw mód)
  renderer.{hpp,cpp}   vykreslení mřížky a skóre
  main.cpp             vstupní bod
tests/
  test_board.cpp       unit testy herní logiky (Catch2)
CMakeLists.txt
```

Veškerá pravidla hry jsou v `Board` a nezávisí na I/O — proto se dají snadno
testovat. Začni u `slideLineLeft()` v `src/board.cpp`, to je srdce hry.

## Claude Code examples

The repo also showcases the four project-scoped ways to extend Claude Code —
skills, hooks, MCP servers, and plugins. Each is a separate, minimal example
tied to this codebase. See [docs/claude-code/](docs/claude-code/README.md).
