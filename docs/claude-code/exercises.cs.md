> 🌍 Číst v jazyce: [English](exercises.md) | **Česky**

# Katalog cvičení: jedna featura na každý mechanismus

Prošel jsi všech šest mechanismů rozšíření ([Skills](01-skills.cs.md),
[Hooks](02-hooks.cs.md), [MCP](03-mcp.cs.md), [Pluginy](04-plugins.cs.md),
[Subagenti](05-agents.cs.md), [Workflows](06-workflows.cs.md)). Tenhle katalog
je promění v praxi: **každá herní featura níže je prostředkem k procvičení
jednoho konkrétního mechanismu.** Nejde jen o featuru — jde o to dostat
mechanismus do ruky.

Každé cvičení je **soběstačné**: dává ti připravený **startovací prompt**, přesné
**soubory**, na které sáhneš, **bug** (kde nějaký je) vysvětlený přímo na místě,
**krok-za-krokem návod s kompletním řešením** a **kontrolu hotovo**. Odkazovaný
návod je nepovinná hloubka.

## Jak používat tento katalog

1. **Vyber řádek** z přehledové tabulky níže.
2. **Otevři odpovídající návod** k danému mechanismu, pokud chceš *jak* a *proč* —
   jinak ho přeskoč; cvičení obstojí samo.
3. **`Shift+Tab`** přepne Claude Code do plan nebo accept-edits režimu (podle
   toho, jak chceš edity revidovat nebo automaticky aplikovat).
4. **Vlož startovací prompt** ze cvičení doslova.
5. **Zaseklý, nebo chceš porovnat?** Každé cvičení končí **walkthroughem** —
   kompletní řešení, krok za krokem, s kódem k vložení.
6. **Spusť příkaz kontroly hotovo** pro potvrzení, že to funguje.

Každé cvičení uvádí svůj **cíl**, **mechanismus**, který učí, **soubory**, na
které sáhneš, připravený **startovací prompt** a **kontrolu hotovo**. Kontrola
hotovo vždy zahrnuje tento základ — projekt se pořád sestaví a sada je zelená:

```bash
cmake -S . -B build && cmake --build build -j && ctest --test-dir build --output-on-failure
```

**Nejsnazší rozjezd:** tohle repo přichází se dvěma skutečnými, záměrnými bugy
(oba použité ve Cvičení 5). Jejich oprava skoro nepotřebuje nový kód a je
nejrychlejší způsob, jak mechanismus pocítit:

- **`isGameOver()`** (`src/board.cpp`, řádky 122–133) kontroluje jen prázdnou
  buňku, takže plná deska, která pořád má mergeable pár, je chybně hlášena jako
  game over.
- **Příznak `changed`** v `src/game.cpp` (řádek 31) se spočítá z `board_.move(...)`
  a pak se ignoruje, takže `spawnRandom()` běží bezpodmínečně (řádek 34) —
  dlaždice se objeví i po tahu, který nic nezměnil.

Obtížnost: 🟢 snadné · 🟡 střední · 🔴 těžké.

## V kostce

| Mechanismus | Cvičení (featura) | Obtížnost | Sahá na |
|---|---|---|---|
| **Skill** | Skill `renderer-style`, pak barevné dlaždice + počítadlo tahů | 🟢–🟡 | `renderer.{hpp,cpp}`, `game.{hpp,cpp}` |
| **Hook** | Stop hook s `ctest`, který zablokuje na červené, pak „konfigurovatelná výherní hodnota" za ním | 🟡 | `.claude/settings.json`, `board.{hpp,cpp}` |
| **MCP** | Trvalé high-score přes MCP server `memory` | 🟡 | `game.cpp`, `renderer.{hpp,cpp}`, `.mcp.json` |
| **Plugin** | Undo workflow zabalený jako bundled skill `/2048-dev:undo` | 🟡 | `plugins/2048-dev/` |
| **Subagent** | Undo featura: naplánovaná, otestovaná, zrevidovaná agenty; oprav oba bugy | 🟢–🟡 | `board`, `game`, `input` |
| **Workflow** | AI řešitel: paralelní heuristiky, benchmark, výběr porotou | 🔴 | nový `ai.{hpp,cpp}`, `board` |

---

## 1. Skill 🟢–🟡 — nauč konvence rendereru a pak je použij

**Cíl.** Napiš nový skill `renderer-style`, který zachytí ANSI / terminálové
kreslicí konvence projektu (escape kódy v `renderer.cpp`, zarovnání `kCellWidth`,
hlavičku se skóre). Pak, *s aktivním skillem*, přidej **barevné dlaždice** (jiná
ANSI barva podle hodnoty dlaždice) a **počítadlo tahů** v hlavičce.

**Co učí.** Tvorbu [skillu](01-skills.cs.md) — popis, který se spustí na správné
požadavky, tělo, jež Clauda zasvětí do konvencí — a sledování, jak se načte na
vyžádání. Srovnej s `board-tests`: stejný tvar, nová doména.

**Soubory.** `.claude/skills/renderer-style/SKILL.md` (nový), `src/renderer.{hpp,cpp}`,
`src/game.{hpp,cpp}` (počítadlo tahů patří na třídu `Game`, **ne** na `Board` —
`Board` zůstává bez I/O).

**Kontext na místě.** `src/renderer.cpp` maže obrazovku přes `\x1b[2J\x1b[H`
(řádek 16), tiskne hlavičku `  2048  —  score:` (řádek 17) a vykresluje mřížku
jako `kCellWidth` široké zprava zarovnané sloupce přes
`std::printf("%*d", kCellWidth, value)` (řádky 11, 25). `Renderer::draw` teď bere
jen `const Board&` (`src/renderer.hpp:13`) — aby zobrazil počítadlo tahů, předáš
počet jako nový argument, protože `Board` žádné počítadlo nemá a nesmí dostat I/O.

**Startovací prompt.**
> *„Vytvoř skill v `.claude/skills/renderer-style/SKILL.md`, který zdokumentuje
> terminálové kreslicí konvence v `src/renderer.cpp`: escape sekvenci
> `\x1b[2J\x1b[H` pro smazání a návrat kurzoru, hlavičku `  2048  —  score:` a
> `kCellWidth` široké zprava zarovnané sloupce tištěné přes
> `std::printf("%*d", kCellWidth, value)`. `description` napiš jako spouštěč pro
> jakoukoli práci s rendererem / ANSI výstupem v tomhle repu."*

Pak, jakmile je skill aktivní:
> *„Se skillem renderer-style přidej každé dlaždici ANSI barvu podle hodnoty a do
> hlavičky počítadlo `moves: N`. `Board` drž bez I/O: počet tahů ulož na třídu
> `Game` v `src/game.{hpp,cpp}` a předej ho do `Renderer::draw` v
> `src/renderer.{hpp,cpp}` vedle desky."*

**Walkthrough (kompletní řešení).**

1. **Napiš skill** v `.claude/skills/renderer-style/SKILL.md`:

```markdown
---
name: renderer-style
description: Terminal-drawing conventions for this 2048 repo — ANSI escape codes, the score header, and kCellWidth column alignment. Use for any work on src/renderer.cpp or other ANSI/terminal output.
---

# Renderer style

This project draws the board to stdout with raw ANSI escape codes. Match these
conventions in any renderer work:

- **Clear + home:** begin a frame with `\x1b[2J\x1b[H`.
- **Header:** one line — `  2048  —  score: <n>` — then a blank line.
- **Grid:** each tile is a `kCellWidth`-wide, right-aligned column printed with
  `std::printf("%*d", kCellWidth, value)`; empty cells print `" ."`.
- **Footer:** the hint `  Arrows / WASD to move,  q to quit`.
- **Colour:** wrap a value in an SGR colour with `\x1b[<code>m … \x1b[0m`; always
  reset with `\x1b[0m` so colour never leaks past the cell.
- Keep all of this in `Renderer`; `Board` stays I/O-free, so the move count is
  passed into `Renderer::draw`.
```

2. **Rozšiř signaturu `draw`**, aby renderer dostal počet tahů
   (`src/renderer.hpp`):

```cpp
    // Clears the screen and renders the board, score and move counter.
    void draw(const Board& board, int moves) const;
```

3. **Přidej tabulku barev a počítadlo** v `src/renderer.cpp` — `colorFor` do
   anonymního namespace, `moves` do hlavičky, každou dlaždici obal do barvy:

```cpp
namespace {
constexpr int kCellWidth = 6;  // characters per tile, e.g. "  2048"

// ANSI SGR colour for each tile value; bigger tiles get hotter colours.
const char* colorFor(int value) {
    switch (value) {
        case 2:
            return "\x1b[37m";  // white
        case 4:
            return "\x1b[36m";  // cyan
        case 8:
            return "\x1b[32m";  // green
        case 16:
            return "\x1b[33m";  // yellow
        case 32:
            return "\x1b[35m";  // magenta
        case 64:
            return "\x1b[31m";  // red
        case 128:
        case 256:
        case 512:
            return "\x1b[94m";  // bright blue
        default:
            return "\x1b[91m";  // bright red (1024+)
    }
}
}  // namespace

void Renderer::draw(const Board& board, int moves) const {
    // Clear screen and move the cursor to the top-left corner.
    std::cout << "\x1b[2J\x1b[H";
    std::cout << "  2048  —  score: " << board.score() << "   moves: " << moves
              << "\n\n";

    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            const int value = board.at(r, c);
            if (value == 0) {
                std::printf("%*s", kCellWidth, " .");
            } else {
                // Colour the tile, then reset so colour never leaks past it.
                std::printf("%s%*d\x1b[0m", colorFor(value), kCellWidth, value);
            }
        }
        std::cout << "\n\n";
    }

    std::cout << "  Arrows / WASD to move,  q to quit\n";
    std::cout.flush();
}
```

4. **Počet ulož na `Game`, ne na `Board`** — přidej k privátním členům v
   `src/game.hpp`:

```cpp
    int moves_ = 0;
```

5. **Počítej skutečné tahy a předej je** (`src/game.cpp`) — inkrementuj jen když
   se deska změnila, pak překresli s počtem:

```cpp
void Game::run() {
    renderer_.draw(board_, moves_);

    while (true) {
        const Command cmd = input_.next();
        if (cmd == Command::Quit) {
            break;
        }
        if (cmd == Command::None) {
            continue;
        }

        const bool changed = board_.move(toDirection(cmd));
        if (changed) {
            ++moves_;
            board_.spawnRandom();
        }
        renderer_.draw(board_, moves_);
```

   Ohraničení spawnu přes `if (changed)` zároveň opraví spawn po prázdném tahu,
   který Cvičení 5 pojmenovává jako bug.

**Hotovo.** Build + `ctest` zelené; hra vykresluje barevné dlaždice a počítadlo
`moves: N`. (Vykreslování drž mimo `Board` — barvy i počet tahů tečou přes
`Renderer`/`Game`.)

## 2. Hook 🟡 — udělej testy nepřeskočitelné, pak přidej hlídanou featuru

**Cíl.** Přidej hook, který spustí `ctest` a **zablokuje na červené** — `Stop`
hook, aby session nemohla skončit s padajícími testy. Pak postav
**konfigurovatelnou výherní hodnotu** (např. hrát do 1024 nebo 4096 místo 2048)
a nech hook zaručit, že nikdy neskončíš s rozbitou sadou.

**Co učí.** [Hook](02-hooks.cs.md), který vynucuje něco *pokaždé*, bez uvážení
modelu — doplněk ke stávajícímu formátovacímu hooku.

**Soubory.** `.claude/hooks/gate-tests.sh` (nový), `.claude/settings.json`
(registrace pod `hooks.Stop`), `src/board.{hpp,cpp}` (`kWinValue` se stane
konfigurovatelným).

**Kontext na místě.** `.claude/hooks/run-tests.sh` už spouští `ctest` na `Stop`,
ale je *poradní*: vypíše řádek pass/fail a vždy `exit 0`. Blokující hook místo
toho **skončí nenulově** (nebo vypíše `{"decision": "block", "reason": "..."}` na
stdout), když testy padnou. Výherní hodnota je natvrdo
`static constexpr int kWinValue = 2048;` na `src/board.hpp:41`.

**Startovací prompt.**
> *„Vytvoř `.claude/hooks/gate-tests.sh` a `chmod +x` ho. Vymodeluj ho podle
> stávajícího `.claude/hooks/run-tests.sh`, ale místo vždy `exit 0` ho nech
> **blokovat**: když `ctest` nahlásí padající test, skonči nenulově (nebo vypiš
> `{"decision": "block", "reason": "tests are red"}` na stdout), aby `Stop`
> nemohl session ukončit na červené. Zaregistruj ho pod `hooks.Stop`
> v `.claude/settings.json` vedle stávající položky `run-tests.sh`."*

Pak, s hlídačem na místě:
> *„Udělej výherní hodnotu konfigurovatelnou. `kWinValue` je
> `static constexpr int kWinValue = 2048;` na `src/board.hpp:41` — nech hrát do
> 1024 nebo 4096, protaženo skrz `Board` bez přidání jakéhokoli I/O."*

**Walkthrough (kompletní řešení).**

1. **Napiš blokující hook** v `.claude/hooks/gate-tests.sh`, pak
   `chmod +x .claude/hooks/gate-tests.sh`:

```bash
#!/usr/bin/env bash
# Stop hook: run the suite and BLOCK the Stop when it is red, so a session can't
# end on failing tests. Models run-tests.sh, but emits a block decision instead
# of always exiting 0.
set -euo pipefail

cat >/dev/null  # drain the Stop-event JSON on stdin

project_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}"
build_dir="$project_dir/build"

# Nothing built yet — let the Stop through.
if [ ! -d "$build_dir" ] || ! command -v ctest >/dev/null 2>&1; then
    exit 0
fi

if ctest --test-dir "$build_dir" >/dev/null 2>&1; then
    exit 0
fi

# Tests are red: block the Stop and tell Claude how to proceed.
echo '{"decision": "block", "reason": "Tests are failing — run ctest --test-dir build --output-on-failure and fix them before stopping."}'
exit 0
```

2. **Zaregistruj ho** pod `hooks.Stop` v `.claude/settings.json`, vedle stávajícího
   poradního `run-tests.sh`:

```json
    "Stop": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/run-tests.sh"
          },
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/gate-tests.sh"
          }
        ]
      }
    ]
```

3. **Udělej z výherní hodnoty člen instance** (`src/board.hpp`) — nahraď natvrdo
   zadanou konstantu `kWinValue` defaultem plus členem a oběma konstruktorům přidej
   parametr `winValue`:

```cpp
    // Starts an empty board and seeds it with two random tiles. The target
    // tile defaults to 2048 but can be set (e.g. play to 1024 or 4096).
    explicit Board(int winValue = kDefaultWinValue);

    // Builds a board from an explicit grid (handy for tests). No tiles are
    // spawned, so the state is fully deterministic.
    explicit Board(Grid grid, int score = 0, int winValue = kDefaultWinValue);
```

```cpp
    bool hasWon() const;  // true once any tile reaches winValue()

    int score() const { return score_; }
    const Grid& grid() const { return grid_; }
    int at(int row, int col) const { return grid_[row][col]; }
    int winValue() const { return winValue_; }

    // Default target tile; the active value lives in winValue_ so it can be
    // changed per game without adding any I/O to Board.
    static constexpr int kDefaultWinValue = 2048;

private:
    Grid grid_{};
    int score_ = 0;
    int winValue_ = kDefaultWinValue;
    std::mt19937 rng_;
```

4. **Protáhni ji konstruktory a `hasWon`** (`src/board.cpp`):

```cpp
Board::Board(int winValue) : winValue_(winValue), rng_(std::random_device{}()) {
    spawnRandom();
    spawnRandom();
}

Board::Board(Grid grid, int score, int winValue)
    : grid_(grid), score_(score), winValue_(winValue), rng_(std::random_device{}()) {}
```

```cpp
            if (value >= winValue_) {
                return true;
            }
```

5. **Nech `Game` hodnotu vybrat** (`src/game.hpp`, pak `src/game.cpp`):

```cpp
    explicit Game(int winValue = Board::kDefaultWinValue);
```

```cpp
Game::Game(int winValue) : board_(winValue) {}
```

   …a po výhře nahlas skutečný cíl:

```cpp
            renderer_.message("You reached " + std::to_string(board_.winValue()) +
                              "! Keep going or press q.");
```

6. **Předej ji z příkazové řádky** (`src/main.cpp`):

```cpp
#include <cstdlib>

#include "game.hpp"

int main(int argc, char** argv) {
    // Optional first argument overrides the win value, e.g. ./2048 1024
    int winValue = g2048::Board::kDefaultWinValue;
    if (argc > 1) {
        winValue = std::atoi(argv[1]);
    }

    g2048::Game game(winValue);
    game.run();
    return 0;
}
```

7. **Přidej regresní test** (`tests/test_board.cpp`):

```cpp
TEST_CASE("hasWon respects a custom win value") {
    Board board(Grid{{{1024, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}},
                0, 1024);
    REQUIRE(board.hasWon());
}
```

   Teď `./build/2048 1024` vyhrává na 1024 a s červenými testy `Stop` hook
   nedovolí session skončit.

**Hotovo.** Záměrně rozbij test → `Stop` hook zablokuje. Oprav → session skončí
čistě. Build + `ctest` zelené s novou výherní hodnotou.

## 3. MCP 🟡 — uchovej high-score napříč sessionami

**Cíl.** Použij už nakonfigurovaný MCP server **`memory`** (viz `.mcp.json`)
k uložení a načtení nejlepšího skóre, aby přežilo mezi běhy. Claude čte/zapisuje
high-score přes MCP server; hra zobrazí `best: N`.

**Co učí.** Dát Claudovi [schopnost](03-mcp.cs.md), kterou nativně nemá (trvalá
paměť), přes MCP server — a propojit jeho data s aplikací.

**Soubory.** `src/game.cpp` (čti/zapisuj nejlepší skóre v `Game::run`),
`src/renderer.{hpp,cpp}` (hlavička, která zobrazí `best: N`), `.mcp.json`
(`memory` už tam je; ověř připojení přes `/mcp`).

**Kontext na místě.** `.mcp.json` už registruje `memory` stdio server
(`@modelcontextprotocol/server-memory`). Hlavička se skóre se kreslí
v `src/renderer.cpp` (řádek 17) a `Renderer::draw` bere jen `const Board&`
(`src/renderer.hpp:13`) — takže nejlepší skóre se musí předat do `Renderer::draw`,
aby se v hlavičce objevilo.

**Startovací prompt.**
> *„Spusť `/mcp` a ověř, že je server `memory` připojený (už je v `.mcp.json`).
> Pak ho použij k uchování nejlepšího skóre napříč sessionami: v `Game::run`
> (`src/game.cpp`) načti uložené nejlepší skóre na začátku a zapiš ho zpět,
> kdykoli ho aktuální skóre překoná. Zobraz ho jako `best: N` v hlavičce —
> hlavička žije v `src/renderer.cpp`, takže předej nejlepší skóre do
> `Renderer::draw` (`src/renderer.{hpp,cpp}`) vedle desky."*

**Walkthrough (kompletní řešení).**

Trvalost žije v MCP serveru `memory` (Claudovo trvalé úložiště); C++ strana jen
*zobrazuje* nejlepší a *hlásí* finální skóre, aby ho Claude uložil. Potkávají se
přes proměnnou prostředí `BEST_SCORE`.

1. **Ověř server.** Spusť `/mcp` a zkontroluj, že je `memory` připojený (už je
   v `.mcp.json`).

2. **Zobraz `best: N` v hlavičce** (`src/renderer.hpp`, pak `src/renderer.cpp`):

```cpp
    // Clears the screen and renders the board, score and best score.
    void draw(const Board& board, int best) const;
```

```cpp
    std::cout << "  2048  —  score: " << board.score() << "   best: " << best
              << "\n\n";
```

3. **Načti nejlepší z prostředí** (`src/game.hpp` — přidej `int best_ = 0;`
   k privátním členům a konstruktor; pak `src/game.cpp`):

```cpp
    Game();
```

```cpp
#include "game.hpp"

#include <algorithm>
#include <cstdlib>

namespace g2048 {

Game::Game() {
    if (const char* env = std::getenv("BEST_SCORE")) {
        best_ = std::atoi(env);
    }
}
```

4. **Sleduj průběžné nejlepší a nahlas ho na konci** (`src/game.cpp`,
   `Game::run`):

```cpp
void Game::run() {
    renderer_.draw(board_, std::max(best_, board_.score()));

    while (true) {
        const Command cmd = input_.next();
        if (cmd == Command::Quit) {
            break;
        }
        if (cmd == Command::None) {
            continue;
        }

        board_.move(toDirection(cmd));
        board_.spawnRandom();

        const int best = std::max(best_, board_.score());
        renderer_.draw(board_, best);

        if (board_.hasWon()) {
            renderer_.message("You reached 2048! Keep going or press q.");
        }
        if (board_.isGameOver()) {
            const int finalBest = std::max(best_, board_.score());
            renderer_.message("Game over. Final score: " +
                              std::to_string(board_.score()) +
                              "  (best: " + std::to_string(finalBest) + ")");
            break;
        }
    }
}
```

5. **Propoj MCP s proměnnou** — tohle je pointa cvičení, Claude je most:
   - *První běh:* `BEST_SCORE=0 ./build/2048`. Když vypíše finální nejlepší,
     řekni Claudovi *„ulož moje nejlepší 2048 skóre do memory serveru"* — Claude
     zavolá server `memory` (entita `2048-best-score` s hodnotou jako observation).
   - *Další běh:* zeptej se *„jaké je moje nejlepší 2048 skóre?"* — Claude ho
     přečte z `memory` — pak spusť `BEST_SCORE=<ta hodnota> ./build/2048` a
     hlavička ukáže `best: N` přenesené mezi sessionami.

**Hotovo.** Zahraj hru, dosáhni skóre, spusť novou session → nejlepší skóre
přetrvá a zobrazí se jako `best: N`. Build + `ctest` zelené.

## 4. Plugin 🟡 — zabal undo workflow jako bundled skill

**Cíl.** Rozšiř plugin `2048-dev` o nový **bundled skill** —
`plugins/2048-dev/skills/undo/SKILL.md` — který řídí undo workflow (drž historii
stavů desky, vrať poslední tah), dodaný vedle stávajícího skillu `build-test`,
aby celý tým sdílel stejné `/2048-dev:undo`.

**Co učí.** Růst [pluginu](04-plugins.cs.md) — přidání bundled skillu pod
`plugins/2048-dev/skills/`, bump verze, validace — aby celý tým sdílel stejný
workflow. **Skills jsou moderní cesta**: vlastní příkazy se sloučily do Skills
([cheat-sheet](cheatsheet.cs.md) vysvětluje konvergenci command→skill), takže
nové části pluginu stavěj jako skilly, ne jako legacy `commands/` soubory.

**Soubory.** `plugins/2048-dev/skills/undo/SKILL.md` (nový),
`plugins/2048-dev/.claude-plugin/plugin.json` (bump `version` 1.2.0 → 1.3.0).
Ověř přes `claude plugin validate .`.

**Kontext na místě.** Plugin už balí jeden skill v
`plugins/2048-dev/skills/build-test/SKILL.md` a je na `version` `1.2.0`
v `plugins/2048-dev/.claude-plugin/plugin.json`. Složka skillu `skills/undo/` se
stane příkazem `/2048-dev:undo` (prefix je jméno pluginu). Záměrně tu není žádný
adresář `commands/` — použij skill.

**Startovací prompt.**
> *„Rozšiř plugin `2048-dev` o nový bundled skill v
> `plugins/2048-dev/skills/undo/SKILL.md`, vymodelovaný podle stávajícího
> `plugins/2048-dev/skills/build-test/SKILL.md`. Má řídit undo featuru: držet
> historii stavů desky a vracet poslední tah. Zvyš `version`
> v `plugins/2048-dev/.claude-plugin/plugin.json` z `1.2.0` na `1.3.0` a pak spusť
> `claude plugin validate .`. Použij Skill, ne legacy `commands/` soubor."*

**Walkthrough (kompletní řešení).**

1. **Přidej bundled skill** v `plugins/2048-dev/skills/undo/SKILL.md`,
   vymodelovaný podle stávajícího skillu `build-test`. (Undo *kód* je Cvičení 5 —
   tenhle skill jen zabalí workflow, aby celý tým sdílel `/2048-dev:undo`.)

```markdown
---
name: undo
description: Add or drive the 2048 undo feature — keep a history of prior board states and revert the last move. Use when working on undo/history in this repo.
disable-model-invocation: true
---

# Undo

Drive the 2048 undo feature: keep a stack of prior board states and revert the
last move on demand.

1. **History.** Before each board-changing move, push a snapshot (grid + score)
   onto a history stack in `Board`. Cap the depth so it can't grow unbounded.
2. **Revert.** `Board::undo()` pops the last snapshot and restores grid + score;
   it returns false when the history is empty.
3. **Key.** Add a `Command::Undo` value in `src/input.hpp`, map `u` to it in
   `src/input.cpp`, and handle it in `Game::run` — undo, redraw, do **not** spawn
   a tile.
4. **Verify.** `cmake --build build -j && ctest --test-dir build --output-on-failure`.
   Add a test: a move then an undo restores the exact prior grid and score.

Report what changed and the test result.
```

2. **Zvyš verzi pluginu** v
   `plugins/2048-dev/.claude-plugin/plugin.json` (`1.2.0` → `1.3.0`):

```json
  "version": "1.3.0",
```

3. **Zvaliduj plugin**, aby byl nový skill v pořádku a vyhledatelný:

```bash
claude plugin validate .
```

**Hotovo.** `claude plugin validate .` projde; `/2048-dev:undo` se napovídá
a načte. Build + `ctest` zelené.

## 5. Subagent 🟢–🟡 — deleguj undo featuru mezi tři agenty

**Cíl.** Postav skutečnou **undo** featuru (drž historii stavů desky; `u` vrátí
poslední tah) tak, že *každou část deleguješ správnému agentovi z
[Ukázky 5](05-agents.cs.md)*:

- `game-explorer` vystopuje, kde se tahy aplikují a kde by žila historie.
- `board-test-writer` nejdřív napíše testy pro undo chování.
- `cpp-reviewer` zreviduje tvůj diff, než zacommituješ.

Jako rozcvičku oprav **dva známé bugy** (`isGameOver()` a příznak `changed`)
s pomocí `cpp-reviewer` pro potvrzení opravy a systematického debugování pro
promyšlení.

**Co učí.** Použití [subagentů](05-agents.cs.md) jako tým: read-only kartograf,
zapisující tester a read-only recenzent — každý ve svém izolovaném kontextu,
každý se správnými nástroji.

**Soubory.** `src/board.{hpp,cpp}` (historie + revert), `src/game.cpp` (obsluha
undo příkazu), `src/input.{hpp,cpp}` (`src/input.hpp` potřebuje novou hodnotu
`Command` pro undo; `src/input.cpp` namapuje klávesu).

**Kontext na místě (dva bugy).**
- **`isGameOver()`** na `src/board.cpp:122–133` vrací `true`, jakmile nenajde
  prázdnou buňku — nikdy nekontroluje mergeable sousedy, takže plná, ale hratelná
  deska je chybně „game over". Odpovídající test je scaffold TODO na
  `tests/test_board.cpp:105`.
- **Příznak `changed`**: `Game::run` spočítá
  `const bool changed = board_.move(toDirection(cmd));` (`src/game.cpp:31`), ale
  pak volá `board_.spawnRandom();` bezpodmínečně (`src/game.cpp:34`). Takže tah
  bez efektu pořád objeví dlaždici a `changed` je nepoužitá proměnná. Tenhle bug
  žije v `Game::run()`, který **nemá testovací harness**.

**Startovací prompt — bug A (isGameOver).**
> *„Předej scaffold TODO na `tests/test_board.cpp:105` agentovi
> `board-test-writer`: přidej `TEST_CASE` pro plnou desku, která pořád obsahuje
> mergeable pár, a vyžaduj `isGameOver() == false`. Spusť ho a sleduj, jak padne —
> `isGameOver()` v `src/board.cpp` (řádky 122–133) kontroluje jen prázdnou buňku.
> Oprav `isGameOver()` tak, aby deska se stejnými ortogonálními sousedy nebyla
> game over, a pak nech `cpp-reviewer` zkontrolovat diff."*

**Startovací prompt — bug B (příznak changed).**
> *„V `src/game.cpp` `Game::run` spočítá
> `const bool changed = board_.move(toDirection(cmd));` (řádek 31), ale pak volá
> `board_.spawnRandom();` bezpodmínečně (řádek 34), takže tah bez efektu pořád
> objeví dlaždici a `changed` je nepoužitý. Ohraň spawn přes `if (changed)`."*

**Walkthrough (kompletní řešení).**

*Rozcvička — nejdřív oprav dva bugy.*

A. **`isGameOver()`** (`src/board.cpp`) — po skenu prázdných buněk navíc zkontroluj
   stejné ortogonální sousedy, než prohlásíš konec hry:

```cpp
bool Board::isGameOver() const {
    // Any empty cell means a move is still possible.
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            if (grid_[r][c] == 0) {
                return false;
            }
        }
    }
    // A full board is still playable if two orthogonal neighbours are equal.
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            const int v = grid_[r][c];
            if (c + 1 < kSize && grid_[r][c + 1] == v) {
                return false;
            }
            if (r + 1 < kSize && grid_[r + 1][c] == v) {
                return false;
            }
        }
    }
    return true;
}
```

   Scaffold TODO na `tests/test_board.cpp:105` proměň v test, který to dokazuje
   (úkol pro `board-test-writer`):

```cpp
TEST_CASE("a full board with a mergeable pair is not game over") {
    Board board(Grid{{{2, 4, 2, 4}, {4, 2, 4, 2}, {2, 4, 2, 4}, {4, 2, 2, 4}}});
    REQUIRE_FALSE(board.isGameOver());
}
```

B. **Příznak `changed`** (`src/game.cpp`) — dlaždici objev jen když tah desku
   skutečně změnil:

```cpp
        const bool changed = board_.move(toDirection(cmd));
        if (changed) {
            board_.spawnRandom();
        }
        renderer_.draw(board_);
```

*Teď undo featura.* `game-explorer` tě navede na `Board::move` (kde se tah
aplikuje) jako místo, kam stav uložit.

1. **Úložiště historie** (`src/board.hpp`) — přidej includy, deklaraci `undo()`
   a člen pro historii:

```cpp
#include <array>
#include <random>
#include <utility>
#include <vector>
```

```cpp
    // Reverts to the state before the last board-changing move. Returns false
    // when there is nothing left to undo.
    bool undo();
```

```cpp
private:
    Grid grid_{};
    int score_ = 0;
    std::mt19937 rng_;

    // Snapshots (grid + score) captured before each board-changing move.
    std::vector<std::pair<Grid, int>> history_;
```

2. **Snímek při změně a revert** (`src/board.cpp`) — ulož i skóre, pushuj jen když
   se deska změnila, a přidej `undo()`:

```cpp
bool Board::move(Direction dir) {
    const Grid before = grid_;
    const int scoreBefore = score_;
    int gained = 0;
```

```cpp
    score_ += gained;
    const bool changed = grid_ != before;
    if (changed) {
        history_.push_back({before, scoreBefore});
    }
    return changed;
}

bool Board::undo() {
    if (history_.empty()) {
        return false;
    }
    const auto [grid, score] = history_.back();
    history_.pop_back();
    grid_ = grid;
    score_ = score;
    return true;
}
```

3. **Klávesa pro undo** — přidej příkaz (`src/input.hpp`) a namapuj `u`
   (`src/input.cpp`):

```cpp
enum class Command { None, Up, Down, Left, Right, Undo, Quit };
```

```cpp
        case 'u':
        case 'U':
            return Command::Undo;
```

4. **Obsluž ho ve smyčce** (`src/game.cpp`) — hned za blokem
   `if (cmd == Command::None)` proveď undo, překresli a přeskoč spawn:

```cpp
        if (cmd == Command::Undo) {
            board_.undo();
            renderer_.draw(board_);
            continue;
        }
```

5. **Otestuj chování** (`tests/test_board.cpp`) — práce pro `board-test-writer`:

```cpp
TEST_CASE("undo restores the grid and score before the last move") {
    Board board(Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    board.move(Direction::Left);  // -> {4, 0, 0, 0}, score +4
    REQUIRE(board.score() == 4);

    REQUIRE(board.undo());
    REQUIRE(board.grid() ==
            Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    REQUIRE(board.score() == 0);
}

TEST_CASE("undo with no history returns false") {
    Board board(Grid{{{2, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    REQUIRE_FALSE(board.undo());
}
```

   Nakonec předej diff `cpp-reviewer`ovi na read-only kontrolu, než zacommituješ.

**Hotovo.** Nové undo testy procházejí a test `isGameOver` (scaffold
`tests/test_board.cpp:105`) zezelená. Oprava příznaku `changed` **nemá unit
test** — žije v `Game::run()`, mimo testovací harness — takže ji ověř ručně:
varování o nepoužité proměnné `changed` je pryč a tah bez efektu už neobjeví
dlaždici. `cpp-reviewer` nehlásí blockery. Build + `ctest` zelené.

## 6. Workflow 🔴 — AI řešitel vybraný porotou

**Cíl.** Přidej AI, která hraje 2048, postavenou a vybranou
[workflow](06-workflows.cs.md). Naimplementuj několik heuristik — corner-stacking,
greedy-merge, monotonicity — a pak napiš workflow, které **generuje/benchmarkuje
každou heuristiku paralelně přes N her** a **porotou** vybere vítěze. (Jednodušší
varianta: fan-out refaktor „konfigurovatelná velikost desky", kde každý agent
upraví jeden soubor, aby se z `kSize` stal parametr.)

**Co učí.** Deterministickou [orchestraci více agentů](06-workflows.cs.md):
fan-out přes `parallel()`, benchmark fázi, fázi poroty/redukce — vzorec
generuj-a-vyber.

**Soubory.** `src/ai.{hpp,cpp}` (nový — drž řešitel čistý jako `Board`),
`src/board.{hpp,cpp}` (bezhlavá cesta „aplikuj tah" pro self-play),
`.claude/workflows/solver-benchmark.js` (nový workflow pro výběr řešitele).

**Kontext na místě.** `.claude/workflows/test-coverage-audit.js` je kompletní,
funkční příklad přesně toho tvaru, který potřebuješ: blok `meta` s fázemi,
fan-out přes `parallel(...)`, strukturovaná output schémata pro každého agenta
a finální redukční fáze. Použij ho jako výchozí model — zkopíruj jeho strukturu
a vyměň read-only `Explore` agenty za build/benchmark kroky.

**Startovací prompt.**
> *„Použij `.claude/workflows/test-coverage-audit.js` jako šablonu (zkopíruj jeho
> tvar `meta`/`phase`/`agent`/strukturované schéma). Napiš nový workflow
> `.claude/workflows/solver-benchmark.js`, který postaví několik 2048 heuristik —
> corner-stacking, greedy-merge, monotonicity — spustí je paralelně přes
> `parallel()` přes N self-play her a pak přidá fázi poroty/redukce, která je
> seřadí a vybere vítěze."*

**Walkthrough (kompletní řešení).**

1. **Deklaruj řešitel** (`src/ai.hpp`) — čistý jako `Board`, bez I/O:

```cpp
#pragma once

#include "board.hpp"

namespace g2048 {

// The heuristics the auto-solver can play with.
enum class Heuristic { CornerStacking, GreedyMerge, Monotonicity };

// Scores a grid under a heuristic; higher is better.
int evaluate(const Grid& grid, Heuristic h);

// Picks the legal move the heuristic rates highest. Pure: it copies the board
// to look one move ahead and performs no I/O.
Direction chooseMove(const Board& board, Heuristic h);

// Plays one headless self-play game with the heuristic and returns the final
// score. Spawns tiles exactly like the interactive game.
int playGame(Heuristic h);

}  // namespace g2048
```

2. **Naimplementuj heuristiky a self-play** (`src/ai.cpp`). `chooseMove` zkusí
   každý směr na *kopii* desky a nechá nejlepší legální; `playGame` odehraje celou
   hru bezhlavě:

```cpp
#include "ai.hpp"

namespace g2048 {

namespace {

int sumTiles(const Grid& grid) {
    int sum = 0;
    for (const auto& row : grid) {
        for (int v : row) {
            sum += v;
        }
    }
    return sum;
}

int countEmpty(const Grid& grid) {
    int empty = 0;
    for (const auto& row : grid) {
        for (int v : row) {
            if (v == 0) {
                ++empty;
            }
        }
    }
    return empty;
}

// Reward big tiles pinned to one corner (the classic 2048 strategy).
int cornerScore(const Grid& grid) {
    static const int weight[kSize][kSize] = {
        {15, 14, 13, 12},
        {8, 9, 10, 11},
        {7, 6, 5, 4},
        {0, 1, 2, 3},
    };
    int score = 0;
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            score += grid[r][c] * weight[r][c];
        }
    }
    return score;
}

// Reward keeping the board empty (more room == more future merges).
int greedyMergeScore(const Grid& grid) {
    return countEmpty(grid) * 100 + sumTiles(grid);
}

// Reward rows and columns that stay ordered, so equal tiles line up to merge.
int monotonicityScore(const Grid& grid) {
    int ordered = 0;
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c + 1 < kSize; ++c) {
            if (grid[r][c] >= grid[r][c + 1]) {
                ++ordered;
            }
        }
    }
    for (int c = 0; c < kSize; ++c) {
        for (int r = 0; r + 1 < kSize; ++r) {
            if (grid[r][c] >= grid[r + 1][c]) {
                ++ordered;
            }
        }
    }
    return ordered * 100 + countEmpty(grid) * 10;
}

}  // namespace

int evaluate(const Grid& grid, Heuristic h) {
    switch (h) {
        case Heuristic::CornerStacking:
            return cornerScore(grid);
        case Heuristic::GreedyMerge:
            return greedyMergeScore(grid);
        case Heuristic::Monotonicity:
            return monotonicityScore(grid);
    }
    return 0;
}

Direction chooseMove(const Board& board, Heuristic h) {
    Direction best = Direction::Left;
    int bestScore = 0;
    bool found = false;
    for (Direction dir :
         {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
        Board copy = board;
        if (!copy.move(dir)) {
            continue;  // illegal: the board did not change
        }
        const int score = evaluate(copy.grid(), h);
        if (!found || score > bestScore) {
            found = true;
            bestScore = score;
            best = dir;
        }
    }
    return best;
}

int playGame(Heuristic h) {
    Board board;
    while (!board.isGameOver()) {
        const Direction dir = chooseMove(board, h);
        if (!board.move(dir)) {
            break;  // no legal move improves anything
        }
        board.spawnRandom();
    }
    return board.score();
}

}  // namespace g2048
```

3. **Bezhlavý benchmark binárka** (`src/bench.cpp`), na kterou se workflow napojí:

```cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ai.hpp"

using namespace g2048;

// Headless benchmark: plays N self-play games with one heuristic and prints
// the average and best score. Usage: ./2048-bench <heuristic> <games>
//   heuristic = corner-stacking | greedy-merge | monotonicity
int main(int argc, char** argv) {
    Heuristic h = Heuristic::CornerStacking;
    int games = 50;

    if (argc > 1) {
        if (std::strcmp(argv[1], "greedy-merge") == 0) {
            h = Heuristic::GreedyMerge;
        } else if (std::strcmp(argv[1], "monotonicity") == 0) {
            h = Heuristic::Monotonicity;
        }
    }
    if (argc > 2) {
        games = std::atoi(argv[2]);
    }

    long total = 0;
    int best = 0;
    for (int i = 0; i < games; ++i) {
        const int score = playGame(h);
        total += score;
        if (score > best) {
            best = score;
        }
    }

    std::printf("heuristic=%s games=%d avg=%.1f best=%d\n",
                argc > 1 ? argv[1] : "corner-stacking", games,
                games > 0 ? double(total) / games : 0.0, best);
    return 0;
}
```

4. **Zapoj nové soubory do buildu** (`CMakeLists.txt`) — přidej `src/ai.cpp` do
   `game_core` a executable `2048-bench`:

```cmake
add_library(game_core
    src/ai.cpp
    src/board.cpp
    src/renderer.cpp
    src/input.cpp
    src/game.cpp)
target_include_directories(game_core PUBLIC src)

# The playable executable.
add_executable(2048 src/main.cpp)
target_link_libraries(2048 PRIVATE game_core)

# Headless benchmark harness for the auto-solver heuristics.
add_executable(2048-bench src/bench.cpp)
target_link_libraries(2048-bench PRIVATE game_core)
```

5. **Otestuj čistou logiku řešitele** — přidej `tests/test_ai.cpp` a zaregistruj
   ho v `tests/CMakeLists.txt` (`add_executable(unit_tests test_board.cpp test_ai.cpp)`):

```cpp
#include <catch2/catch_test_macros.hpp>

#include "ai.hpp"

using namespace g2048;

TEST_CASE("greedy-merge prefers an emptier board") {
    Grid full{{{2, 4, 2, 4}, {4, 2, 4, 2}, {2, 4, 2, 4}, {4, 2, 4, 2}}};
    Grid sparse{{{2, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};
    REQUIRE(evaluate(sparse, Heuristic::GreedyMerge) >
            evaluate(full, Heuristic::GreedyMerge));
}

TEST_CASE("chooseMove returns a legal move for a playable board") {
    Board board(Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    const Direction dir = chooseMove(board, Heuristic::GreedyMerge);
    REQUIRE(board.move(dir));  // the chosen move actually changes the board
}
```

6. **Workflow pro výběr** (`.claude/workflows/solver-benchmark.js`) — zkopírovaný
   z tvaru `test-coverage-audit.js`: blok `meta`, fan-out přes `parallel()` se
   schématy pro každého agenta a fáze poroty/redukce:

```js
export const meta = {
  name: 'solver-benchmark',
  description:
    'Build the 2048 auto-solver, benchmark each heuristic over N self-play games, and judge-pick the strongest.',
  phases: [
    { title: 'Build', detail: 'configure + build the 2048-bench harness once' },
    { title: 'Benchmark', detail: 'one agent per heuristic runs N self-play games' },
    { title: 'Judge', detail: 'rank heuristics by score and pick a winner' },
  ],
}

// The heuristics in src/ai.cpp, with the CLI name 2048-bench expects.
const HEURISTICS = [
  { key: 'corner-stacking', note: 'pin the biggest tile to a corner' },
  { key: 'greedy-merge', note: 'keep the board as empty as possible' },
  { key: 'monotonicity', note: 'keep rows and columns ordered' },
]

const GAMES = 200  // self-play games per heuristic

const RESULT_SCHEMA = {
  type: 'object',
  properties: {
    heuristic: { type: 'string' },
    games: { type: 'number' },
    avg: { type: 'number', description: 'average final score across the games' },
    best: { type: 'number', description: 'best single-game score' },
  },
  required: ['heuristic', 'avg', 'best'],
}

const RANKING_SCHEMA = {
  type: 'object',
  properties: {
    winner: { type: 'string', description: 'the strongest heuristic key' },
    ranking: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          heuristic: { type: 'string' },
          rank: { type: 'number' },
          avg: { type: 'number' },
          rationale: { type: 'string' },
        },
        required: ['heuristic', 'rank', 'avg'],
      },
    },
    markdown: { type: 'string', description: 'the rendered comparison table' },
  },
  required: ['winner', 'ranking', 'markdown'],
}

// ── Phase 1: build the benchmark harness once ──────────────────────────────
phase('Build')
await agent(
  `In this 2048 C++ repo, configure and build so the benchmark binary exists: ` +
    `run "cmake -S . -B build" then "cmake --build build -j". Confirm ` +
    `./build/2048-bench was produced. Report only OK or the first build error.`,
  { label: 'build:bench', phase: 'Build' }
)

// ── Phase 2: benchmark each heuristic in parallel ──────────────────────────
// A barrier (parallel) is correct: the judge needs every result at once.
phase('Benchmark')
const results = await parallel(
  HEURISTICS.map((h) => () =>
    agent(
      `Run "./build/2048-bench ${h.key} ${GAMES}" in this repo. It prints one ` +
        `line like "heuristic=${h.key} games=${GAMES} avg=1823.1 best=3200". ` +
        `Parse and return the heuristic name, games, avg and best as numbers.`,
      { label: `bench:${h.key}`, phase: 'Benchmark', schema: RESULT_SCHEMA }
    )
  )
)
const scores = results.filter(Boolean)

// ── Phase 3: judge the results and pick a winner ───────────────────────────
phase('Judge')
const ranking = await agent(
  `You are judging three 2048 auto-solver heuristics by their benchmark scores. ` +
    `Rank them best-to-worst by average score (break ties with best score), name ` +
    `the winner, and render a Markdown table with columns: rank | heuristic | avg ` +
    `| best. RESULTS:\n${JSON.stringify(scores, null, 2)}`,
  { label: 'judge:rank', phase: 'Judge', schema: RANKING_SCHEMA }
)

log(`Benchmarked ${scores.length} heuristics; winner: ${ranking.winner}.`)
return ranking.markdown
```

   Spusť ho přes `/solver-benchmark`. Rychlá kontrola z shellu —
   `./build/2048-bench greedy-merge 30` — už vypíše skutečný řádek
   `avg=… best=…` pro každou heuristiku, který porota seřadí.

**Hotovo.** Auto-řešitel odehraje celou hru bezhlavě; workflow proběhne a vydá
seřazené srovnání. Build + `ctest` zelené, včetně testů nové čisté logiky
řešitele.

---

Vyber libovolný řádek, vlož jeho startovací prompt a postav to. Každé cvičení tě
nechá s funkční featurou *a* mechanismem, který sis sám vyzkoušel.
