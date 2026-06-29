> 🌍 Číst v jazyce: [English](exercises.md) | **Česky**

# Katalog cvičení: jedna featura na každý mechanismus

Prošel jsi všech šest mechanismů rozšíření ([Skills](01-skills.cs.md),
[Hooks](02-hooks.cs.md), [MCP](03-mcp.cs.md), [Pluginy](04-plugins.cs.md),
[Subagenti](05-agents.cs.md), [Workflows](06-workflows.cs.md)). Tenhle katalog
je promění v praxi: **každá herní featura níže je prostředkem k procvičení
jednoho konkrétního mechanismu.** Nejde jen o featuru — jde o to dostat
mechanismus do ruky.

Každé cvičení uvádí svůj **cíl**, **mechanismus**, který učí, **soubory**, na
které sáhneš, a **kontrolu hotovo**. Kontrola hotovo je vždy stejný základ:
projekt se pořád sestaví a sada je zelená —

```bash
cmake -S . -B build && cmake --build build -j && ctest --test-dir build --output-on-failure
```

**Nejsnazší rozjezd:** tohle repo přichází se dvěma skutečnými, záměrnými bugy.
Jejich oprava skoro nepotřebuje nový kód a je nejrychlejší způsob, jak
mechanismus pocítit:

- **`isGameOver()`** (`src/board.cpp`) kontroluje jen prázdnou buňku, takže plná
  deska, která pořád má mergeable pár, je chybně hlášena jako game over.
- **Příznak `changed`** v `src/game.cpp` se spočítá z `board_.move(...)` a pak se
  ignoruje, takže dlaždice se objeví i po tahu, který nic nezměnil.

Obtížnost: 🟢 snadné · 🟡 střední · 🔴 těžké.

## V kostce

| Mechanismus | Cvičení (featura) | Obtížnost | Sahá na |
|---|---|---|---|
| **Skill** | Skill `renderer-style`, pak barevné dlaždice + počítadlo tahů | 🟢–🟡 | `renderer.cpp`, `board.{hpp,cpp}` |
| **Hook** | Pre-commit/Stop hook s `ctest`, pak „konfigurovatelná výherní hodnota" za ním | 🟡 | `.claude/settings.json`, `board.{hpp,cpp}` |
| **MCP** | Trvalé high-score přes MCP server `memory` | 🟡 | `game.cpp`, `.mcp.json` |
| **Plugin** | Undo + replay jako příkazy `/2048-dev:` | 🟡 | `plugins/2048-dev/` |
| **Subagent** | Undo featura: naplánovaná, otestovaná, zrevidovaná agenty; oprav oba bugy | 🟢–🟡 | `board`, `game`, `input` |
| **Workflow** | AI řešitel: paralelní heuristiky, benchmark, výběr porotou | 🔴 | nový `ai.{hpp,cpp}`, `board` |

---

## 1. Skill — nauč konvence rendereru a pak je použij

**Cíl.** Napiš nový skill `renderer-style`, který zachytí ANSI / terminálové
kreslicí konvence projektu (escape kódy v `renderer.cpp`, zarovnání `kCellWidth`,
hlavičku se skóre). Pak, *s aktivním skillem*, přidej **barevné dlaždice** (jiná
ANSI barva podle hodnoty dlaždice) a **počítadlo tahů** v hlavičce.

**Co učí.** Tvorbu [skillu](01-skills.cs.md) — popis, který se spustí na správné
požadavky, tělo, jež Clauda zasvětí do konvencí — a sledování, jak se načte na
vyžádání. Srovnej s `board-tests`: stejný tvar, nová doména.

**Soubory.** `.claude/skills/renderer-style/SKILL.md` (nový), `src/renderer.cpp`,
`src/board.{hpp,cpp}` (počet tahů patří do herního stavu).

**Hotovo.** Build + `ctest` zelené; hra vykresluje barevné dlaždice a počítadlo
tahů. (Vykreslování drž mimo `Board` — barvy patří do `Renderer`.)

## 2. Hook — udělej testy nepřeskočitelné, pak přidej hlídanou featuru

**Cíl.** Přidej hook, který spustí `ctest` a **zablokuje na červené** — buď
`Stop` hook (aby session nemohla skončit s padajícími testy), nebo pre-commit
hook. Pak postav **konfigurovatelnou výherní hodnotu** (např. hrát do 1024 nebo
4096 místo 2048) a nech hook zaručit, že ji nikdy nezacommituješ s rozbitou
sadou.

**Co učí.** [Hook](02-hooks.cs.md), který vynucuje něco *pokaždé*, bez uvážení
modelu — doplněk ke stávajícímu formátovacímu hooku.

**Soubory.** `.claude/settings.json` (+ skript pod `.claude/hooks/`),
`src/board.{hpp,cpp}` (`kWinValue` se stane konfigurovatelným).

**Hotovo.** Záměrně rozbij test → hook zablokuje. Oprav → projde. Build + `ctest`
zelené s novou výherní hodnotou.

## 3. MCP — uchovej high-score napříč sessionami

**Cíl.** Použij už nakonfigurovaný MCP server **`memory`** (viz `.mcp.json`)
k uložení a načtení nejlepšího skóre, aby přežilo mezi běhy. Claude čte/zapisuje
high-score přes MCP server; hra zobrazí „best: N".

**Co učí.** Dát Claudovi [schopnost](03-mcp.cs.md), kterou nativně nemá (trvalá
paměť), přes MCP server — a propojit jeho data s aplikací.

**Soubory.** `src/game.cpp` (zobraz nejlepší skóre), `.mcp.json` (`memory` už tam
je; ověř připojení přes `/mcp`).

**Hotovo.** Zahraj hru, dosáhni skóre, spusť novou session → nejlepší skóre
přetrvá. Build + `ctest` zelené.

## 4. Plugin — zabal undo + replay jako sdílitelné příkazy

**Cíl.** Rozšiř plugin `2048-dev` o vývojářské příkazy: `/2048-dev:undo`
a `/2048-dev:replay` (příkaz, který nastartuje nebo řídí undo/replay featuru),
plus příkaz na **scaffolding featury**, který vygeneruje kostru nové metody
`Board` s odpovídajícím testem.

**Co učí.** Růst [pluginu](04-plugins.cs.md) — víc příkazů v
`plugins/2048-dev/commands/`, bump verze, validace — aby celý tým sdílel stejné
příkazy.

**Soubory.** `plugins/2048-dev/commands/*.md` (nové),
`plugins/2048-dev/.claude-plugin/plugin.json` (bump verze). Ověř přes
`claude plugin validate .`.

**Hotovo.** `claude plugin validate .` projde; nové příkazy se napovídají
a běží. Build + `ctest` zelené.

## 5. Subagent — deleguj undo featuru mezi tři agenty

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
undo příkazu), `src/input.cpp` (namapuj klávesu).

**Hotovo.** Nové testy pro undo procházejí; oba testy známých bugů procházejí;
`cpp-reviewer` nehlásí blockery. Build + `ctest` zelené.

## 6. Workflow — AI řešitel vybraný porotou

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
`.claude/workflows/*.js` (workflow pro výběr řešitele).

**Hotovo.** Auto-řešitel odehraje celou hru bezhlavě; workflow proběhne a vydá
seřazené srovnání. Build + `ctest` zelené, včetně testů nové čisté logiky
řešitele.

---

Vyber libovolný řádek, přečti odpovídající návod a postav to. Každé cvičení tě
nechá s funkční featurou *a* mechanismem, který sis sám vyzkoušel.
