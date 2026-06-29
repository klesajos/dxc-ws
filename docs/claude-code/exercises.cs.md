> 🌍 Číst v jazyce: [English](exercises.md) | **Česky**

# Katalog cvičení: jedna featura na každý mechanismus

Prošel jsi všech šest mechanismů rozšíření ([Skills](01-skills.cs.md),
[Hooks](02-hooks.cs.md), [MCP](03-mcp.cs.md), [Pluginy](04-plugins.cs.md),
[Subagenti](05-agents.cs.md), [Workflows](06-workflows.cs.md)). Tenhle katalog
je promění v praxi: **každá herní featura níže je prostředkem k procvičení
jednoho konkrétního mechanismu.** Nejde jen o featuru — jde o to dostat
mechanismus do ruky.

Každé cvičení je **soběstačné**: dává ti připravený **startovací prompt**, přesné
**soubory**, na které sáhneš, **bug** (kde nějaký je) vysvětlený přímo na místě
a **kontrolu hotovo**. Odkazovaný návod je nepovinná hloubka.

## Jak používat tento katalog

1. **Vyber řádek** z přehledové tabulky níže.
2. **Otevři odpovídající návod** k danému mechanismu, pokud chceš *jak* a *proč* —
   jinak ho přeskoč; cvičení obstojí samo.
3. **`Shift+Tab`** přepne Claude Code do plan nebo accept-edits režimu (podle
   toho, jak chceš edity revidovat nebo automaticky aplikovat).
4. **Vlož startovací prompt** ze cvičení doslova.
5. **Spusť příkaz kontroly hotovo** pro potvrzení, že to funguje.

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

**Hotovo.** Auto-řešitel odehraje celou hru bezhlavě; workflow proběhne a vydá
seřazené srovnání. Build + `ctest` zelené, včetně testů nové čisté logiky
řešitele.

---

Vyber libovolný řádek, vlož jeho startovací prompt a postav to. Každé cvičení tě
nechá s funkční featurou *a* mechanismem, který sis sám vyzkoušel.
