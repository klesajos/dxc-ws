> 🌍 Číst v jazyce: [English](10-validate-eval.md) | **Česky**

# Ukázka 10: Validace a evaluace pluginu

## Co je validace a co jsou evaly?

Ukázky 1–9 rozšíření *stavějí*. Tahle je *kontroluje*. Claude Code má čtyři
vestavěné nástroje na kontrolu kvality, od nejlevnějšího po nejdůkladnější:

| Nástroj | Na jakou otázku odpovídá | Stojí tokeny? |
|------|---------------------|---------------|
| `claude plugin validate <path>` | Je plugin strukturálně platný: manifest, frontmatter, odkazy na soubory? | Ne |
| `claude plugin details <name>` | Co je v pluginu a kolik stojí v kontextu? | Ne |
| `/skill-doctor` | Které načtené skills se nikdy nepoužijí a kolik stojí? | Ne |
| `claude plugin eval <path>` | Dělá plugin Clauda ve své práci opravdu **lepším**? | Ano, spouští skutečné session |

**Eval** je pro plugin obdoba unit testu. Napíšeš prompt a k němu
**gradery**, které Claudovu odpověď ohodnotí. `claude plugin eval` spustí prompt
v hermetickém sandboxu **s** tvým pluginem a **bez** něj a pak ukáže
rozdíl (Δ). Kladné Δ je důkaz, že si plugin svou cenu v kontextu
zaslouží.

## Co dělá tahle ukázka

Sada evalů `plugins/2048-dev/evals/explorer-trace/` kontroluje agenta pluginu
`game-explorer` ([Ukázka 5](05-agents.cs.md)). Požádá Clauda, ať vystopuje,
jak se šipka doleva dostane až k `Board::move`, a odpověď pak ohodnotí třemi
způsoby. Ověřený běh na Claude Code 2.1.283 získal **1.00 s pluginem
oproti 0.33 bez něj** (Δ +0.67, jeden běh na větev, $0.24).

## Soubory řádek po řádku

```
plugins/2048-dev/
└── evals/                      ← default eval directory of a plugin
    └── explorer-trace/         ← one directory per case
        ├── case.yaml           ← case metadata + setup
        ├── prompt.md           ← run settings (frontmatter) + the prompt (body)
        ├── setup.sh            ← scaffold script: prepares the workspace
        ├── fixture/            ← trimmed copy of the 2048 call chain
        └── graders/            ← one file per grader
            ├── criteria.md
            ├── explorer-used.md
            └── reads-input.md
```

**`case.yaml`** — povinný, kdykoli potřebuješ pole `context.*`:

```yaml
schema_version: "1.1"
name: explorer-trace
context:
  scaffold_script: setup.sh
```

`schema_version` a `name` jsou v `case.yaml` povinné. Bez nich se běh
nenačte.

**`setup.sh`** — každý běh začíná v **prázdném** dočasném workspace
bez přístupu k tvému repu. Scaffold skript do něj nakopíruje fixture:

```bash
#!/usr/bin/env bash
# Runs in the empty eval workspace before Claude starts (only with --scaffold).
# Copies the trimmed 2048 call chain into the workspace as src/.
set -euo pipefail
mkdir -p src
cp "$(dirname "$0")"/fixture/*.cpp src/
```

**`prompt.md`** — frontmatter nastavuje běh, tělo se odešle doslova:

```markdown
---
description: The game-explorer agent maps a keypress down to Board::move
max_turns: 15
allowed_tools: [Read, Glob, Grep, Agent]
tags: [smoke, agent]
---

The source code of a terminal 2048 game is in `src/`.
Trace how pressing the Left arrow key ends up calling `Board::move`.
Answer with an ordered list of `file:function` steps, from `main()` to
`Board::move`.
```

`allowed_tools` uvádí jen nástroje pro čtení, takže žádné povolení přes
`--allow-tools` není potřeba. `Bash`, `Edit`, `Write` a `WebFetch` jsou
chráněné a musí se povolit na příkazové řádce.

**Tři gradery** používají tři různé typy graderů:

```markdown
---
type: llm
weight: 2
---

PASS if the answer is an ordered list that starts at `main` (main.cpp), goes
through the game loop in game.cpp and the key reading in input.cpp, and ends at
`Board::move` in board.cpp, naming a file and a function at each step.
FAIL if any of those four files is missing, the order is wrong, or the answer
invents functions that are not in the source.
```

```markdown
---
type: tool_used
tool: Agent
input_match: "game-explorer"
arm: with-only
---

The plugin's game-explorer agent was delegated to.
```

```markdown
---
type: regex
pattern: "input\\.cpp"
weight: 1
---

The trace must pass through input.cpp, where the arrow key is decoded. A
reply that only says "I couldn't find Board::move" never names this file,
so it fails here.
```

- `llm` — model v roli soudce (výchozí je Haiku) hlasuje třikrát. PASS potřebuje
  dva hlasy ze tří. Rubriku piš jako konkrétní podmínky PASS a FAIL.
- `tool_used` s `arm: with-only` — „spustil se agent pluginu?" Větev
  bez pluginu ho spustit nemůže, takže tenhle grader se vypisuje, ale **nezapočítává
  se do skóre**.
- `regex` — zdarma a deterministický. **Poučení ze stavby téhle sady:**
  první verze hledala `Board::move` a prošla i odpovědí, která říkala
  „I can't find `Board::move`". Zvol vzor, který obsahuje jen *správná* odpověď.

## Vytvoř si vlastní eval, krok za krokem

1. **Vytvoř prázdný case** z kořene pluginu:
   ```bash
   cd plugins/my-plugin
   claude plugin eval init --bare my-case
   ```
   Tohle zapíše `evals/my-case/prompt.md` a `evals/my-case/graders/criteria.md`.
   Bez `--bare` dostaneš interaktivní rozhovor, který s tebou gradery
   navrhne.

2. **Napiš prompt** do `prompt.md`. Formuluj ho tak, jak by to řekl skutečný uživatel.
   Nejmenuj v něm svůj skill ani agenta — eval má dokázat, že si ho Claude
   vybere sám.

3. **Napiš aspoň jeden grader** do `graders/`. Dávej přednost levným,
   deterministickým graderům (`regex`, `tool_used`, `file_exists`) a přidej jeden
   `llm` grader na kvalitu.

4. **Potřebuješ ve workspace soubory?** Přidej `case.yaml` s
   `context.scaffold_script` a spouštěj s `--scaffold`.

5. **Při iterování to spouštěj levně:**
   ```bash
   claude plugin eval . --scaffold --runs 1 --no-publish
   ```

6. **Ignoruj adresář s výsledky** (`.gitignore` tohohle repa to už dělá):
   ```gitignore
   plugins/*/evals/results/
   ```

## Vyzkoušej demo

Nejdřív validuj. Je to zdarma a rychlé:

```bash
claude plugin validate plugins/2048-dev --json
claude plugin details 2048-dev
```

`details` ukáže účet pluginu za kontext: asi 460 tokenů trvale načtených, skoro
všechno z toho je popis `game-explorer`.

Pak spusť eval. Dělá skutečná volání API, zhruba $0.25 za pár běhů:

```bash
cd plugins/2048-dev
claude plugin eval . --scaffold --runs 1 --no-publish
```

Očekávaný výstup (skóre se mezi běhy liší):

```text
✓ explorer-trace  with 1.00  without 0.33  Δ +0.67  (2 runs)  $0.24
```

První běh tě požádá o potvrzení, že pluginu důvěřuješ. V CI předej
`--trust-plugin`. Otevři cestu k `report.html` vypsanou na konci — najdeš tam
verdikty jednotlivých graderů a důkazy soudce.

## Volitelné parametry

| Přepínač | Co dělá |
|------|--------------|
| `--runs <n>` | Počet běhů na větev (výchozí: `runs` z case, jinak 3). Víc běhů znamená méně zašuměné skóre |
| `--ablation none` | Přeskočí větev bez pluginu. Poloviční cena, ale bez Δ |
| `--scaffold` | Spustí `scaffold_script`. Výchozí je vypnuto, protože spouští tvůj bash pod tvým účtem |
| `--allow-tools <tools...>` | Povolí chráněné nástroje, např. `Bash(cmake:*)` |
| `--threshold <0..1>` | Skončí kódem 1, když některý case skóruje pod touto hranicí (výchozí 1.0). Použij v CI |
| `--max-cost-usd <usd>` | Pevný rozpočet. Když se dosáhne, vypíšou se dílčí výsledky |
| `--judge-model <model>` | Model pro `llm` gradery (výchozí Haiku) |
| `--no-publish` | Nechá HTML report lokálně místo soukromého publikování na claude.ai |
| `--json [path]` | Kompletní strojově čitelný výsledek |

Další typy graderů: `file_exists`, `tool_order` a `baseline`. Skills postavené
na MCP se dají evaluovat proti **mock** serverům v `evals/mocks/`. Úplná
reference: [oficiální dokumentace plugin evals](https://code.claude.com/docs/en/plugin-evals).

## Kde to funguje: CLI, Desktop aplikace, Cowork

| Platforma | Funguje? | Nastavení |
|----------|--------|-------|
| **Claude Code CLI** (terminál) | ✅ Ano | `claude plugin validate`, `details` a `eval` jsou podpříkazy CLI. `/skill-doctor` běží uvnitř session |
| **Claude Desktop — záložka Code** | ⚠️ Částečně | Podpříkazy `claude plugin …` jsou jen v CLI. Spusť je z terminálu ve stejném repu |
| **Cowork** (v Desktop aplikaci) | ❌ Ne | Plugin evaluuj v CLI, než ho nainstaluješ do Coworku |

## Když něco nefunguje

- **`missing required field schema_version`** nebo **`name: Required`** →
  `case.yaml` potřebuje `schema_version: "1.1"` i `name`.
- **Claude tvrdí, že soubory nenajde** → workspace začíná prázdný.
  Použij `scaffold_script` a spouštěj s `--scaffold`. Běh nemůže číst
  adresář s evaly ani následovat symlinky ven z něj.
- **Δ je 0 nebo záporné** → plugin se nespouští (zkontroluj grader `with-only`)
  nebo je základní varianta v úloze už sama dobrá. Tak či tak možná
  plugin nestojí za svou trvalou cenu v kontextu.
- **Skóre mezi běhy skáče** → to je variabilita modelu. Než uděláš
  závěry, zvyš `--runs`.
