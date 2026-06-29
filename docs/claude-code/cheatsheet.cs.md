> 🌍 Číst v jazyce: [English](cheatsheet.md) | **Česky**

# Claude Code tahák

Číslované průvodce tě učí, jak Claude Code **rozšířit** ([skills](01-skills.cs.md),
[hooks](02-hooks.cs.md), [MCP](03-mcp.cs.md), [pluginy](04-plugins.cs.md),
[subagenti](05-agents.cs.md), [workflows](06-workflows.cs.md)). Tahle stránka je
ta druhá půlka: jak ho
**ovládat** každý den — přepínače, slash příkazy, klávesové zkratky, prefixy
a události hooků, po kterých sáhneš pořád, ale nezaslouží si každý vlastní
průvodce.

> ✅ **Ověřeno proti Claude Code 2.1.195 (2026-06-29).** Rozhraní CLI se mezi
> verzemi mění — zkontroluj `claude --version` a
> [oficiální dokumentaci](https://code.claude.com/docs/en/cli-reference), pokud
> tu něco nesedí.

## Spouštěcí přepínače (`claude ...`)

| Příkaz | Co dělá |
|--------|---------|
| `claude` | Spustí interaktivní session v aktuální složce |
| `claude "oprav lint chyby"` | Spustí session s prvním promptem už ve frontě |
| `claude -c` / `--continue` | Obnoví **poslední** session v této složce |
| `claude -r` / `--resume [id]` | Obnoví **konkrétní** session (bez id → interaktivní výběr) |
| `claude -p "..."` / `--print` | **Headless**: spustí prompt, vypíše výsledek, skončí. Pro skripty / CI |
| `claude --model opus` | Spustí na konkrétním modelu (`opus`, `sonnet`, `haiku`, `opusplan`) |
| `claude --agent cpp-reviewer` | Spustí celou session jako pojmenovaného subagenta |
| `claude --add-dir ../lib` | Dá session přístup k dalším složkám mimo repo |

## Slash příkazy

Napiš `/` pro automatické doplnění. Seskupené podle účelu:

**Session a kontext**

| Příkaz | Co dělá |
|--------|---------|
| `/clear` | Vymaže konverzaci — čistý štít pro novou úlohu |
| `/compact` | Shrne dosavadní konverzaci a uvolní kontext (zachová podstatu) |
| `/context` | Ukáže, co zrovna plní kontextové okno |
| `/rewind` | Skočí na dřívější checkpoint (kód i/nebo konverzaci) — taky `Esc Esc` |
| `/resume` | Přepne na jinou minulou session bez opuštění |
| `/export` | Uloží celou konverzaci do Markdown souboru |

**Konfigurace**

| Příkaz | Co dělá |
|--------|---------|
| `/config` | Otevře menu nastavení (model, téma, oprávnění) |
| `/model` | Přepne model uprostřed session |
| `/permissions` | Zobrazí / udělí trvalá oprávnění nástrojům |
| `/memory` | Upraví trvalou paměť (soubory `CLAUDE.md`) |
| `/init` | Vygeneruje startovní `CLAUDE.md` pro tohle repo |
| `/status`, `/statusline` | Přehled stavu session / úprava spodní info lišty |
| `/doctor` | Diagnostika rozbité instalace (auth, síť, závislosti) |

**Rozšíření** (šest mechanismů, které pokrývají průvodce)

| Příkaz | Co dělá |
|--------|---------|
| `/skill-name` | Spustí skill, např. `/board-tests` (Ukázka 1) |
| `/2048-dev:build-test` | Spustí skill zabalený v pluginu (Ukázka 4) |
| `/agents` | Vytvoří / spravuje subagenty |
| `/hooks` | Zobrazí / spravuje hooky |
| `/mcp` | Zobrazí MCP servery a autorizuje je |
| `/plugin` | Instaluje / spravuje pluginy a marketplaces |

**Info**

| Příkaz | Co dělá |
|--------|---------|
| `/help` | Vypíše všechny příkazy, zkratky a funkce |
| `/usage` | Zbývající kapacita na tarifu Pro/Max |
| `/cost` | Útrata v USD / tokenech za aktuální session (API billing) |

## Klávesové zkratky

| Klávesa | Co dělá |
|---------|---------|
| `Ctrl+C` | Přeruší aktuální akci (nouzový vypínač) |
| `Ctrl+R` | Zpětné hledání v historii promptů/příkazů |
| `Esc` | Zruší aktuální vstup / zavře menu |
| `Esc Esc` | Otevře **Rewind** — obnoví dřívější checkpoint |
| `Shift+Tab` | Cykluje režimy oprávnění (default → accept-edits → plan) |
| `Ctrl+G` | Otevře plánovací soubor v editoru (v režimu Plan) |

## Vstupní prefixy

| Prefix | Co dělá |
|--------|---------|
| `!` | **Bash režim** — spustí shellový příkaz přímo, bez modelu, bez tokenů (např. `!git status`) |
| `@` | **Zmínka souboru** — vtáhne konkrétní soubor/složku do kontextu (doplňuje se) |
| `#` | **Přidat do paměti** — uloží řádek do `CLAUDE.md` na příště |
| `\` + Enter | **Víceřádkový vstup** — pokračuj na novém řádku bez odeslání |

## Události hooků

Kam se hook může navěsit v životním cyklu session (viz [Ukázka 2](02-hooks.md)):

| Událost | Spustí se | Typické použití |
|---------|-----------|-----------------|
| `SessionStart` | Začátek session | Načtení kontextu, kontroly prostředí |
| `UserPromptSubmit` | Odeslání promptu | Přidání kontextu, validace/přepis promptu |
| `PreToolUse` | Před spuštěním nástroje | **Zablokovat** nebezpečné akce (např. `git push --force`) |
| `PostToolUse` | Po úspěšném nástroji | Formát / lint — *hook `format-cpp` v tomhle repu* |
| `Stop` | Claude dokončí odpověď | Ověřit testy — *hook `run-tests` v tomhle repu* |
| `SessionEnd` | Konec session | Úklid, archivace, reporting |

## Režimy oprávnění (cykluj `Shift+Tab`)

| Režim | Claude smí… |
|-------|-------------|
| **default** | Ptát se před každou úpravou a příkazem |
| **acceptEdits** | Volně upravovat soubory, u shell příkazů se pořád ptá |
| **plan** | Jen pro čtení — bádá a sepíše plán, nic nemění |
| **bypassPermissions** | Cokoli bez ptaní („YOLO" — jen v sandboxu) |

Dva další režimy existují mimo běžný cyklus `Shift+Tab` — **auto** (jedná
s bezpečnostními kontrolami na pozadí) a **dontAsk** (jen předem schválené
nástroje); viz [dokumentace režimů oprávnění](https://code.claude.com/docs/en/permission-modes).

## Modely a effort

| Model | Nejlepší na |
|-------|-------------|
| `opus` | Nejtěžší úvahy — architektura, záludné bugy |
| `sonnet` | Denní tahoun — featury, refaktory, testy |
| `haiku` | Rychlý a levný — boilerplate, rychlé kontroly, paralelní subagenti |
| `opusplan` | Opus na plán, pak se automaticky přepne na Sonnet na implementaci |

Hloubku úvah lad' přes **effort** (`low` / `medium` / `high`, některé modely
přidávají vyšší stupně) přes `/model` nebo nastavení. Vyšší effort = hlubší
přemýšlení, víc tokenů, pomalejší.

## Ekonomika kontextu (proč na `/compact` záleží)

- Claude čte **celou** konverzaci každý tah, takže cena roste s historií.
  `/compact` shrne, aby zůstala štíhlá; `/clear` resetuje pro novou úlohu.
- **Prompt caching** dává velkou slevu na stabilní prefix (tvůj `CLAUDE.md`,
  struktura projektu) — je automatický; drž ten prefix stabilní.
- **Pozor na rozšíření:** každý aktivní MCP server, skill a subagent ujídá
  kontext. Zapni to, co úloha potřebuje, ne všechno.

## Co je v roce 2026 legacy (sjednocení)

Vlastní slash příkazy byly **sloučeny do Skills** (Ukázka 1). Staré soubory
příkazů fungují dál, ale nové věci dělej přes Skills:

| Bývalo samostatné | Teď | Stav |
|-------------------|-----|------|
| Vlastní slash příkazy (`.claude/commands/*.md`) | **Skills** (`.claude/skills/<name>/SKILL.md`) | Sloučeno — obojí vytváří `/name`; legacy soubory fungují dál |

(Output styles zůstávají *samostatnou*, stále aktuální funkcí — mění systémový
prompt, zatímco skills načítají instrukce k úloze — viz
[dokumentace output styles](https://code.claude.com/docs/en/output-styles);
deprecován byl jen samostatný příkaz `/output-style` ve prospěch `/config`.)

Skill je striktní nadmnožina starého příkazu: stejné vyvolání `/name`, **plus**
volitelné autonomní načtení, složka pro pomocné soubory a řízení vyvolání. Chceš,
aby se skill choval jako starý „jen ručně" příkaz? Přidej do frontmatteru
`disable-model-invocation: true`.

Zdroj: [oficiální dokumentace Skills](https://code.claude.com/docs/en/skills) —
*„Custom commands have been merged into skills."*

---

**Měj tenhle tahák po ruce při práci.** Je to reference, ne tutoriál — pro *jak*
a *proč* každého mechanismu rozšíření viz číslované průvodce v
[indexu](README.md).
