> 🌍 Číst v jazyce: [English](08-output-styles.md) | **Česky**

# Ukázka 8: Projektový output style

## Co je output style?

**Output style** mění **to, jak s tebou Claude mluví**, ne to, co umí.
Je to Markdown soubor, jehož tělo se přidá do Claudova systémového promptu.
Srovnej s ostatními mechanismy:

- **Skill** přidává znalosti pro jeden druh úlohy, načítá se na vyžádání.
- **Pravidlo** nebo `CLAUDE.md` přidává fakta a konvence projektu.
- **Output style** formuje *každou* odpověď: její délku, strukturu a tón.

Claude Code obsahuje pět vestavěných stylů:

| Styl | Co dělá |
|------|---------|
| **Default** | Standardní softwarově-inženýrský prompt Claude Code, bez přidaného stylu |
| **Proactive** | Pustí se do práce hned a místo ptaní u rutinních rozhodnutí udělá rozumné předpoklady |
| **Concise** | Začne výsledkem a vynechá úvod, komentování průběhu i rekapitulace |
| **Explanatory** | Přidává krátké bloky `★ Insight`, které vysvětlují volby za kódem |
| **Learning** | Vysvětluje volby a nechává malé kousky kódu na tobě |

`/output-style` byl krátce označen jako deprecated a ve verzi 2.1.269 se
vrátil. Concise přibyl ve verzi 2.1.237.

## Co dělá tahle ukázka

Styl `workshop-tutor` dá každé odpovědi stejný třídílný tvar: **What I did**,
**Why it works** a **Try it**. Celá workshopová místnost tak dostává odpovědi,
které se dají porovnat vedle sebe. Každá odpověď končí příkazem, který si
účastník může spustit, takže nikdo jen pasivně nečte.

## Soubor řádek po řádku

Styl žije v `.claude/output-styles/workshop-tutor.md`:

```
.claude/
└── output-styles/          ← project output styles (committed to git)
    └── workshop-tutor.md   ← file name = style name, unless `name:` overrides it
```

```markdown
---
name: workshop-tutor
description: Explains each change for workshop participants and ends with a hands-on "Try it" step
keep-coding-instructions: true
---

# Workshop tutor

You are pairing with a participant of a Claude Code workshop on the 2048 C++
project. Keep doing the engineering work as usual, but shape every answer like
this:

1. **What I did** — one or two sentences, naming the files and functions you
   touched as `file:line`.
2. **Why it works** — the C++ or Claude Code concept behind the change, in at
   most three bullets. Skip anything a working developer already knows.
3. **Try it** — one concrete command or prompt the participant can run next
   to see the result for themselves (for example `./build/2048` or
   `ctest --test-dir build --output-on-failure`).

Keep the tone plain and direct. No praise, no filler.
```

Co dělá který řádek:

- `name:` — název zobrazený v `/output-style` a `/config`. Nepovinné; když
  ho vynecháš, použije se název souboru.
- `description:` — zobrazí se vedle názvu ve výběru.
- `keep-coding-instructions: true` — **nejdůležitější řádek.** Vlastní styl
  *nahrazuje* vestavěné softwarově-inženýrské instrukce Claude Code (jak
  vymezit rozsah změn, psát komentáře, ověřovat práci), pokud tohle
  nenastavíš. Chceme, aby Claude dál kódoval stejně, jen jinak mluvil, takže
  je to zapnuté.
- Tělo je instrukce, kterou se Claude řídí u každé odpovědi.

## Vytvoř si vlastní output style, krok za krokem

1. **Vytvoř složku:**
   ```bash
   mkdir -p .claude/output-styles
   ```

2. **Vytvoř soubor** `.claude/output-styles/my-style.md`:
   ```markdown
   ---
   description: <one line shown in the picker>
   keep-coding-instructions: true
   ---

   <How every answer should look: structure, length, tone.>
   ```

3. **Restartuj Claude Code.** Soubory stylů se čtou při startu, takže styl,
   který vytvoříš nebo upravíš během session, se objeví až po restartu.

4. **Přepni se na něj:**
   ```text
   /output-style my-style
   ```
   Spusť `/output-style` bez argumentu a uvidíš seznam všech stylů. Aktuální
   je označený. Nový styl platí od tvé **další** zprávy.

5. **Commitni ho**, aby si ho mohl vybrat celý tým:
   ```bash
   git add .claude/output-styles/my-style.md
   git commit -m "Add my-style output style"
   ```
   Commit dělá styl *dostupným*, ne *aktivním*. Každý si ho vybírá sám.
   Menu `/config` uloží tu volbu do `.claude/settings.local.json`, tvého
   osobního souboru, který zůstává mimo git.

## Vyzkoušej demo

```text
/output-style workshop-tutor
```

Pak se zeptej: *„Co vrací Board::hasWon()? Nic neupravuj."*

Odpověď přijde ve třech označených částech a skončí příkazem **Try it**,
třeba `grep -n "hasWon" tests/test_board.cpp`. Zpátky se přepneš přes
`/output-style default`.

Stejnou kontrolu spustíš i bez interaktivní session:

```bash
claude -p --settings '{"outputStyle":"workshop-tutor"}' \
  "What does Board::hasWon() return? Don't edit anything."
```

## Volitelné parametry

| Pole / nastavení | Kde | Co dělá |
|------------------|-----|---------|
| `name` | frontmatter stylu | Zobrazovaný název. Výchozí: název souboru |
| `description` | frontmatter stylu | Text zobrazený ve výběru |
| `keep-coding-instructions` | frontmatter stylu | `true` zachová vestavěné inženýrské instrukce Claude Code. Výchozí `false` |
| `force-for-plugin` | frontmatter stylu, **jen styly z pluginů** | `true` zapne styl vždy, když je plugin povolený, a přebije volbu uživatele |
| `outputStyle` | libovolný `settings.json` | Vybere styl bez menu. **Záleží na velikosti písmen**: `"Explanatory"`, ne `"explanatory"`. Neodpovídající hodnota spadne zpět na Default |

Styly se načítají z `~/.claude/output-styles/` (osobní), z každé
`.claude/output-styles/` mezi pracovním adresářem a kořenem repa
(projektové) a z povolených pluginů. Úplná reference:
[oficiální dokumentace output styles](https://code.claude.com/docs/en/output-styles).

## Kde to funguje: CLI, Desktop aplikace, Cowork

| Platforma | Funguje? | Nastavení |
|-----------|----------|-----------|
| **Claude Code CLI** (terminál) | ✅ Ano | `/output-style <name>`, nebo `/config` → **Output style** |
| **Claude Desktop — záložka Code** | ✅ Ano | Nastav `outputStyle` v `.claude/settings.local.json`. V Desktopu `/config` místo menu otevře **Settings > Claude Code** |
| **Rozšíření pro VS Code** | ✅ Ano | `/` → **Output styles**, včetně vlastních stylů (2.1.257+) |
| **Cowork** (v Desktop aplikaci) | ❌ Ne | Projektová konfigurace `.claude/` se nenačítá. Dodej styl místo toho v pluginu |

## Když něco nefunguje

- **Styl není v seznamu `/output-style`** → soubor musí být
  v `.claude/output-styles/` s příponou `.md`. Po vytvoření restartuj
  Claude Code.
- **Claude po přepnutí přestal spouštět testy nebo začal dělat ledabylé
  úpravy** → chybí `keep-coding-instructions: true`, takže styl nahradil
  inženýrské instrukce.
- **`outputStyle` v nastavení nemá žádný efekt** → zkontroluj velikost
  písmen. Hodnota musí přesně odpovídat názvu stylu.
