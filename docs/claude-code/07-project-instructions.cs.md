> 🌍 Číst v jazyce: [English](07-project-instructions.md) | **Česky**

# Ukázka 7: Projektová pravidla omezená na cesty

## Co jsou projektové instrukce?

**Projektové instrukce** jsou Markdown soubory, které Claude čte jako stálé
pokyny pro repozitář. Jsou tři druhy a liší se tím, *kdy* se načítají:

- `CLAUDE.md` (nebo `.claude/CLAUDE.md`) se načítá na začátku **každé**
  session. Drž ho krátký: build příkazy, konvence, struktura.
- `.claude/rules/*.md` **bez** klíče `paths:` se taky načítají při startu
  session. Slouží k rozdělení dlouhého `CLAUDE.md` do tematických souborů.
- `.claude/rules/*.md` **s** klíčem `paths:` se načtou **jen když Claude čte
  nebo upravuje odpovídající soubor**. Právě to učí tahle ukázka.

`AGENTS.md`, soubor s instrukcemi napříč nástroji, který používají jiní
coding agenti, se čte taky, ale ve výchozím stavu **jen když v pracovním
adresáři ani nad ním není `CLAUDE.md`**. Tohle repo `CLAUDE.md` má, takže
`AGENTS.md` by tu byl ignorován, dokud nezměníš nastavení **Project
instructions** v `/config`. (Přímé čtení `AGENTS.md` vyžaduje Claude Code
2.1.277 nebo novější.)

## Co dělá tahle ukázka

Pravidlo `board-logic` nese omezení, na kterých záleží **jen když někdo sahá
na herní pravidla**: `Board` musí zůstat bez I/O, testy musí používat
deterministický konstruktor, potom musí proběhnout `ctest`. Účastník, který
pracuje na rendereru, za tyhle řádky nikdy neplatí kontextem. Ve chvíli, kdy
Claude otevře `src/board.cpp` nebo `src/board.hpp`, se načtou.

## Soubor řádek po řádku

Pravidlo žije v `.claude/rules/board-logic.md`:

```
.claude/              ← project-scoped Claude Code config (committed to git)
└── rules/            ← every .md file in here is a rule (searched recursively)
    └── board-logic.md  ← file name is free; only the .md extension matters
```

```markdown
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
```

Co dělá která část:

- `paths:` — seznam glob vzorů. **Je to jediný klíč frontmatteru, který
  pravidlo má.** S ním je pravidlo *omezené na cesty*: načte se, když Claude
  použije Read, Write nebo Edit na odpovídající soubor, ne při každém volání
  nástroje. Bez něj se pravidlo načte při startu session jako `CLAUDE.md`.
- Globy jsou relativní ke kořenu projektu. Funguje i `src/**/*.cpp` nebo
  `tests/**`.
- Tělo je obyčejný Markdown. Piš ho jako `CLAUDE.md`: krátce, v rozkazovacím
  způsobu, konkrétně k souborům, na které je pravidlo omezené.

## Vytvoř si vlastní pravidlo, krok za krokem

1. **Vytvoř složku:**
   ```bash
   mkdir -p .claude/rules
   ```

2. **Vytvoř soubor pravidla**, např. `.claude/rules/tests.md`:
   ```markdown
   ---
   paths:
     - "tests/**"
   ---

   # Test rules

   - One behaviour per TEST_CASE, Arrange-Act-Assert.
   - Never weaken an assertion to make a test pass; report the bug instead.
   ```

3. **Začni novou session.** Pravidla se hledají při startu session.

4. **Ověř, že se načte jen když má:** nejdřív se zeptej na něco
   k `src/renderer.cpp`, pak spusť `/context`. Pravidlo pod **Memory files**
   **není**. Teď požádej Clauda, ať přečte `tests/test_board.cpp`, a spusť
   `/context` znovu. Pravidlo tam je.

5. **Commitni ho:**
   ```bash
   git add .claude/rules/tests.md
   git commit -m "Add path-scoped test rules"
   ```

## Vyzkoušej demo

Zeptej se Clauda: *„Přečti src/board.hpp. Máš v kontextu projektové pravidlo
s nadpisem 'Board logic rules'? Ocituj jeho první odrážku."*

Claude ocituje odrážku o I/O. Polož stejnou otázku v nové session **bez**
předchozího čtení souboru boardu a Claude pravidlo nevidí.

## Audit instrukcí: `/doctor prompt-audit`

Soubory s instrukcemi stárnou. Zůstávají v nich formulace psané pro starší
modely, odkazují na příkazy, které už neexistují, nebo si navzájem odporují.
Od verze 2.1.283 `/doctor prompt-audit` (také `/checkup prompt-audit`)
zreviduje tvoje soubory `CLAUDE.md`, `CLAUDE.local.md` a `AGENTS.md` a k tomu
pravidla, skills, příkazy, subagenty a output styly v `.claude/`
a `~/.claude/`. Dostaneš report s navrženými úpravami a nic se nezmění, dokud
Clauda nepožádáš, ať je aplikuje. Audit jen jedné cesty:

```text
/doctor prompt-audit .claude/rules
```

## Volitelné parametry

| Nastavení / klíč | Kde | Co dělá |
|------------------|-----|---------|
| `paths:` | frontmatter pravidla | Seznam globů. Omezí pravidlo na cesty |
| `@path/to/file` | uvnitř `CLAUDE.md` | Importuje jiný soubor do `CLAUDE.md`. Relativní cesty se řeší od importujícího souboru. Max 4 skoky |
| `claudeMdExcludes` | libovolný `settings.json` | Seznam globů absolutních cest, které se přeskočí, např. `CLAUDE.md` jiného týmu v monorepu. Soubory managed policy vyloučit nejde |
| **Project instructions** | `/config` | Volba, jestli číst jen `CLAUDE.md`, i `AGENTS.md`, nebo jen managed instrukce |
| `omitClaudeMd: true` | frontmatter subagenta | Subagent přeskočí uživatelské, projektové i lokální soubory `CLAUDE.md`. Vestavěné Explore a Plan to dělají už teď |

Úplná reference: [oficiální dokumentace memory](https://code.claude.com/docs/en/memory).

## Kde to funguje: CLI, Desktop aplikace, Cowork

| Platforma | Funguje? | Nastavení |
|-----------|----------|-----------|
| **Claude Code CLI** (terminál) | ✅ Ano | Nic navíc. Pravidla v `.claude/rules/` se najdou při startu session |
| **Claude Desktop — záložka Code** | ✅ Ano | Stejný engine jako CLI. Otevři složku projektu a potvrď dialog důvěry |
| **Cowork** (v Desktop aplikaci) | ❌ Ne | Cowork projektovou konfiguraci `.claude/` nenačítá. Dej instrukce místo toho do skillu uvnitř pluginu |

## Když něco nefunguje

- **Pravidlo se v `/context` nikdy neobjeví** → porovnej glob se skutečnou
  cestou (`src/board.cpp`, ne `./src/board.cpp`). Pamatuj, že pravidlo
  omezené na cesty se objeví až **poté**, co Claude přečte nebo upraví
  odpovídající soubor.
- **Pravidlo se načítá v každé session** → frontmatter chybí nebo je rozbitý,
  takže tam není klíč `paths:`. Řádek `---` musí být první řádek souboru.
- **`AGENTS.md` se ignoruje** → očekávané, dokud existuje `CLAUDE.md`. Buď
  ho importuj z `CLAUDE.md` přes `@AGENTS.md`, nebo změň **Project
  instructions** v `/config`.
