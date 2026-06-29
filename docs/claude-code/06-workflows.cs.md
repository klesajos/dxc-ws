> 🌍 Číst v jazyce: [English](06-workflows.md) | **Česky**

# Ukázka 6: Workflows (orchestrace více agentů)

## Co je workflow?

**Workflow** je malý JavaScriptový program, který **orchestruje víc agentů
s deterministickým tokem řízení**. Místo aby ses Clauda ptal, ať „si vymyslí
kroky", kroky napíšeš sám — rozfanouješ práci přes `parallel()`, zřetězíš ji
přes `pipeline()`, seskupíš přes `phase()` — a runtime tu strukturu pokaždé
vykoná přesně tak. Uložené workflow se stane slash příkazem, který spustíš přes
`/<jmeno>`.

Tam, kde si jeden agent volí cestu sám, je cesta workflow pevně daná v kódu:
smyčky, podmínky i fan-out máš pod kontrolou ty.

## Subagent vs. workflow

Tohle jsou dva delegační mechanismy a vyplatí se mít v jejich rozdílu jasno,
než si přečteš kód:

| | Subagent (Ukázka 5) | Workflow (tahle ukázka) |
|---|---|---|
| **Jednotka** | jeden delegovaný kontext | víc agentů, orchestrovaných |
| **Tok řízení** | kroky si volí agent | volíš *ty*, v JavaScriptu (`parallel`/`pipeline`/`phase`) |
| **Determinismus** | řízeno modelem, liší se běh od běhu | struktura je pevná; stejný vstup → stejný tvar běhu |
| **Definováno v** | Markdown souboru + systémový prompt | `.js` skriptu s `export const meta` |
| **Vyvolání** | prostá řeč, `@agent-name`, `--agent` | slash příkaz `/<jmeno>` (opt-in) |

**Základní pravidlo:** sáhni po **subagentovi**, když chceš předat *jeden*
samostatný úkol *jednomu* izolovanému kontextu. Sáhni po **workflow**, když chceš
spustit *víc* agentů v *opakovatelné, kódem definované* pipeline — rozfanoutovat
přes N souborů, podmínit fázi 2 fází 1, zredukovat víc výsledků do jedné zprávy.

## Co tahle ukázka dělá

Tahle ukázka přidává jedno spustitelné workflow, `test-coverage-audit`, které
audituje, jak dobře `tests/test_board.cpp` pokrývá logiku v `src/`. Je **jen
pro čtení, deterministické a opakovatelné** — vypíše prioritizovanou zprávu
o mezerách a nikdy nezapíše soubor, takže ho můžeš spouštět, jak chceš často,
se stabilním výsledkem.

Funguje ve třech fázích:

1. **Inventory** — rozfanouj jednoho read-only čtenáře na každý zdrojový soubor;
   každý vyjmenuje chování, na která může mířit unit test.
2. **Map tests** — jeden agent namapuje každý existující `TEST_CASE` na tenhle
   inventář.
3. **Report gaps** — jeden reduktor obojí zkříží a vydá prioritizovanou zprávu
   o mezerách.

Protože skutečné mezery v tomhle repu jsou známé (TODO `{4, 4, 8, 0}`, distribuce
`spawnRandom`, game-over plné desky s mergeable párem, který naráží na bug
v `isGameOver()`, vícetahové sekvence, ignorovaný příznak `changed` v `game.cpp`),
je zpráva předvídatelná — což z ní dělá dobrý učební artefakt.

## Soubor řádek po řádku

Workflow žije v `.claude/workflows/test-coverage-audit.js`. Uložená workflow
v `.claude/workflows/` se auto-objeví a stanou se slash příkazy.

**1. Blok `meta` musí být úplně první příkaz — na tomhle to každý poprvé
zasekne.** Jakýkoli `import`, `const` nebo spustitelný řádek nad
`export const meta` způsobí, že se workflow **tiše neregistruje**:
`/test-coverage-audit` se vůbec neobjeví, bez chyby, která by vysvětlila proč.
Komentář `//` v hlavičce souboru před `meta` nevadí (tenhle soubor jedním
začíná); spustitelný kód ano. Blok pojmenuje workflow (tím vznikne
`/test-coverage-audit`) a popíše jeho fáze:

```js
export const meta = {
  name: 'test-coverage-audit',
  description: 'Audit src/ test coverage against tests/test_board.cpp and emit a prioritized gap report.',
  phases: [
    { title: 'Inventory', detail: 'one read-only reader per src file enumerates behaviours' },
    { title: 'Map tests', detail: 'map each TEST_CASE onto the behaviour inventory' },
    { title: 'Report gaps', detail: 'cross-reference and emit a prioritized gap report' },
  ],
}
```

**2. Pět zdrojových souborů, které inventarizuje.** Pole `SRC_FILES` jmenuje
každý soubor v `src/`, u každého jednořádkovou poznámku, ať se čtenářský agent
rychle zorientuje. Tohle je doslova těch „pět souborů", přes která se rozfanouje
fáze 1:

```js
const SRC_FILES = [
  { file: 'src/board.cpp', note: 'pure rules: slideLineLeft, move, spawnRandom, isGameOver, hasWon' },
  { file: 'src/game.cpp', note: 'main loop: toDirection, Game::run, spawn + redraw, win/over checks' },
  { file: 'src/input.cpp', note: 'raw-mode terminal: getchar -> Command (WASD + arrow keys)' },
  { file: 'src/renderer.cpp', note: 'draws the grid + score to the terminal' },
  { file: 'src/main.cpp', note: 'entry point: constructs Game, calls run()' },
]
```

**3. Tři schémata nutí každého agenta vrátit data, ne prózu.** Předání `schema`
do `agent()` přiměje runtime výstup agenta zvalidovat, takže skript dostane zpět
skutečný objekt — žádné parsování. Workflow definuje jedno na každou fázi:
`BEHAVIOR_SCHEMA` (co jeden soubor dělá, větev po větvi), `COVERAGE_SCHEMA`
(která id chování pokrývá každý `TEST_CASE`) a `GAP_SCHEMA` (prioritizované
mezery plus vykreslená tabulka). Tady je ukázané jen `BEHAVIOR_SCHEMA`:

```js
const BEHAVIOR_SCHEMA = {
  type: 'object',
  properties: {
    behaviors: { type: 'array', items: { type: 'object', properties: {
      id: { type: 'string' }, file: { type: 'string' }, line: { type: 'number' },
      behavior: { type: 'string' }, pureLogic: { type: 'boolean' }, sideEffects: { type: 'string' },
    }, required: ['id', 'file', 'line', 'behavior', 'pureLogic'] } },
  },
  required: ['behaviors'],
}
```

`GAP_SCHEMA` nese pole `markdown` — a **právě tohle pole nakonec slash příkaz
vrací** (viz krok 5).

**4. Fáze 1 rozfanouje přes `parallel()`, pak fáze 2 namapuje testy — a každý
agent je jen pro čtení.** Každý agent běží jako vestavěný typ **`Explore`**
(`agentType: 'Explore'`); `phase()` jen pojmenuje každou skupinu v UI:

```js
phase('Inventory')
const inventories = await parallel(
  SRC_FILES.map((f) => () =>
    agent(`Read ${f.file} ... enumerate every behaviour a unit test could target ...`,
      { label: `inventory:${f.file}`, phase: 'Inventory', schema: BEHAVIOR_SCHEMA, agentType: 'Explore' })
  )
)
const behaviors = inventories.filter(Boolean).flatMap((r) => r.behaviors)

phase('Map tests')
const coverageResult = await agent(`Read tests/test_board.cpp ... map each TEST_CASE ...`,
  { label: 'map:tests', schema: COVERAGE_SCHEMA, agentType: 'Explore' })
```

`parallel()` je **bariéra**: počká na *všechny* čtenáře, než pokračuje. Tady je
to záměr — fáze 2 potřebuje *celý* inventář, než na něj namapuje testy. (Naopak
`pipeline()` žene každou *položku* všemi fázemi nezávisle; hodí se na řetězce po
položkách jako feature-pipeline níže, ne na fan-in.)

> **Proč je každý agent `Explore`?** Agenti `agentType: 'Explore'` umí číst
> a hledat, ale nemají `Edit`, `Write` ani `Bash`. Právě tohle vynucení — ne
> prosba — dělá `/test-coverage-audit` bezpečným pro spouštění ve smyčce:
> `git status` zůstává pokaždé čistý. Sílu zápisu přidej, jen když přidáš
> i izolaci (`isolation: 'worktree'`).

**5. Redukční prompt je to, co dělá běh deterministickým.** Fáze 3 předá jednomu
reduktoru celý inventář i mapu pokrytí a — to je klíčové — *zafixuje nálezy,
které musí vyplavat* přímo v promptu. Tady je ten prompt fáze 3 doslova
(vypuštěn je jen vkládaný JSON):

```js
phase('Report gaps')
const report = await agent(
  `You are auditing unit-test coverage for a 2048 C++ game. Cross-reference the behaviour ` +
    `inventory against the coverage map and produce a PRIORITIZED gap report. A gap is a ` +
    `behaviour with pureLogic=true that no TEST_CASE covers; prioritize by risk and how core ` +
    `the behaviour is (slide/merge logic is the heart of the game).\n\n` +
    `Known high-value gaps you should expect to surface:\n` +
    `- slideLineLeft on {4, 4, 8, 0} (the TODO in tests/test_board.cpp).\n` +
    `- spawnRandom: returns false on a full board, and its 2-vs-4 distribution.\n` +
    `- a FULL board that still contains a mergeable pair: isGameOver() should return false. ` +
    `Note that isGameOver() in src/board.cpp only checks for empty cells, so a correct test ` +
    `for this case currently FAILS — flag it as a latent bug, do not hide it.\n` +
    `- multi-move sequences (score/state across several moves).\n` +
    `- the 'changed' flag in src/game.cpp is computed but ignored, so a tile spawns even ` +
    `after a no-op move (game-loop behaviour, currently untested).\n\n` +
    `BEHAVIOURS:\n(… the behaviour inventory, injected as JSON …)\n\n` +
    `COVERAGE:\n(… the coverage map, injected as JSON …)\n\n` +
    `Return a 'gaps' array and a 'markdown' field. The markdown must contain a table with ` +
    `columns: behaviour id | file:line | priority | suggested TEST_CASE name | why. This is a ` +
    `read-only audit: recommend tests to add, do not edit anything.`,
  { label: 'reduce:gaps', phase: 'Report gaps', schema: GAP_SCHEMA, agentType: 'Explore' }
)

log(`Audited ${behaviors.length} behaviours; found ${report.gaps.length} coverage gaps.`)
return report.markdown   // pole GAP_SCHEMA.markdown je to, co příkaz zobrazí
```

Zafixování očekávaných mezer v promptu je **proč** opětovné spuštění
`/test-coverage-audit` vyplaví pokaždé stejné nálezy: model je neobjevuje znovu
od nuly, jen potvrzuje známý seznam (a má pokyn označit případ `isGameOver()`
jako *latentní bug*, ne ho schovat). Skriptové `return report.markdown` je přesně
to pole `GAP_SCHEMA.markdown` — co slash příkaz vypíše.

## Pokročilejší tvar: feature-pipeline (popsaná, ne dodaná)

Audit jen čte. Workflow může i bezpečně *pracovat*, pokud ho izoluješ.
`feature-pipeline` na přidání featury by vypadala takhle:

```
design (read-only architekt)
  → vstup do git worktree (izolace)
    → naimplementuj featuru
    → cmake + ctest, s omezeným opakováním při selhání
    → review agentem cpp-reviewer z Ukázky 5   ← agenti se skládají do workflow
  → výstup z worktree
→ předej větev člověku k mergi
```

Dvě myšlenky stojí za zmínku: **worktree** drží práci na izolované kopii repa,
takže selhaný běh nikdy nesáhne na tvůj strom, a review fáze **znovu používá
subagenta `cpp-reviewer`** — agent z Ukázky 5 se stane fází v pipeline Ukázky 6.
Tolik mašinérie se vyplatí, když je změna vícesouborová, hlídaná testy a chceš
souběžnost; na jednořádkovou opravu, kterou bys prostě udělal rovnou, je to
přehnané.

## Vytvoř si vlastní workflow krok za krokem

1. **Vytvoř složku:**
   ```bash
   mkdir -p .claude/workflows
   ```
2. **Vytvoř** `.claude/workflows/my-flow.js` začínající `meta`:
   ```js
   export const meta = { name: 'my-flow', description: 'What it does.', phases: [{ title: 'Work' }] }
   phase('Work')
   const result = await agent('Do one well-scoped thing.', { schema: { type: 'object', properties: { summary: { type: 'string' } }, required: ['summary'] } })
   return result.summary
   ```
3. **Drž ho deterministické a ideálně nejdřív jen pro čtení.** Pro agenty, co mají
   jen číst, použij `agentType: 'Explore'`. Sílu zápisu přidej, až když přidáš
   i izolaci (worktree).
4. **Restartuj Claude Code**, ať se workflow objeví, a spusť `/my-flow`. Spuštění
   workflow je **opt-in** — vyvoláš ho explicitně.
5. **Zacommituj ho:**
   ```bash
   git add .claude/workflows/my-flow.js && git commit -m "Add my-flow workflow"
   ```

## Vyzkoušej demo

1. V čerstvé session `claude` v kořeni repa spusť **`/test-coverage-audit`**.
   Sleduj, jak se rozfanouje přes pět souborů v `src/` (fáze 1), namapuje testy
   (fáze 2) a vypíše prioritizovanou zprávu o mezerách (fáze 3), která pojmenuje
   známé mezery — včetně bugu `isGameOver()` jako latentního problému.
2. Zkontroluj `git status` — je **čistý**. Workflow jen četlo.
3. Spusť ho znovu. *Tvar* je stejný a vyplavou stejné mezery — to je
   determinismus, který ti kódem definovaná pipeline dává.
4. Přečti si sekci „feature-pipeline" výš a vystopuj, kde by worktree izoloval
   práci a kde se `cpp-reviewer` zapojí jako review fáze.

## Volitelné parametry

Volby `agent()`, po kterých sáhneš nejčastěji:

| Volba | Co dělá |
|-------|---------|
| `schema` | JSON Schema, kterému musí výstup agenta odpovídat; `agent()` vrátí zvalidovaný objekt |
| `label` | Jméno agenta v UI průběhu |
| `phase` | Přiřadí agenta do skupiny průběhu (použij uvnitř `parallel`/`pipeline`) |
| `agentType` | Běh jako konkrétní typ agenta, např. `Explore` pro vynucené jen-pro-čtení |
| `model` / `effort` | Přepíše model nebo úsilí uvažování pro fázi |
| `isolation: 'worktree'` | Běh agenta v čerstvém git worktree — pro fáze, co mění soubory |

Orchestrační primitiva: `phase()`, `parallel()` (bariéra), `pipeline()` (řetězce
po položkách, bez bariéry), `log()`. Úplná reference:
[oficiální dokumentace](https://code.claude.com/docs/en/workflows).

## Kde to funguje: CLI, Desktop, Cowork

| Platforma | Funguje? | Nastavení |
|-----------|----------|-----------|
| **Claude Code CLI** (terminál) | ✅ Ano | Workflows v `.claude/workflows/` se objeví automaticky; spusť `/test-coverage-audit` |
| **Desktop — záložka Code** | ✅ Ano* | Stejný engine jako CLI; projektová workflow se načtou po dialogu důvěry. *Ověř, že je funkce workflows ve tvém buildu zapnutá |
| **Cowork** (v Desktop aplikaci) | ❌ Ne* | Cowork projektovou konfiguraci `.claude/` nenačítá, včetně `.claude/workflows/`. *Orchestrace workflow je funkce lokálního enginu; pokud je potřebuješ v Coworku, zabal podkladové agenty do pluginu |

## Řešení problémů

- **`/test-coverage-audit` se neobjeví** → soubor musí být v `.claude/workflows/`
  a mít `export const meta = { ... }` jako první příkaz (komentář `//` v hlavičce před ním nevadí); restartuj session.
- **Agent spadne s „unknown agent type"** → tvůj build možná typ `Explore`
  nevystavuje; odeber `agentType: 'Explore'` a místo toho agentovi v promptu
  napiš, ať jen čte (slabší vynucení, ale poběží).
- **Zpráva se mezi běhy hodně mění** → utáhni prompt a schéma reduktoru; čím víc
  struktury zafixuješ, tím stabilnější výstup.

---

Teď jsi viděl všech šest projektových mechanismů. Kam dál — odstupňované menu
herních featur, každá jako prostředek k procvičení jednoho mechanismu — viz
[katalog cvičení](exercises.cs.md).
