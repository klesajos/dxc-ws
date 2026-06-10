> 🌍 Číst v jazyce: [English](README.md) | **Česky**

# Ukázky rozšíření Claude Code

Tohle repo zároveň slouží jako ukázka čtyř způsobů, jak rozšířit Claude Code
na úrovni **projektu**. „Na úrovni projektu" znamená, že konfigurace žije
**přímo v repozitáři** — když si projekt naklonuješ přes `git clone`,
dostaneš celé nastavení automaticky. Nic se neinstaluje do domovského
adresáře.

Každá ukázka je samostatná a minimální. Projdi je popořadě — jdou od
nejjednodušší po nejpokročilejší:

| # | Ukázka | Co tě naučí | Kde žije |
|---|--------|-------------|----------|
| 1 | [Skills](01-skills.cs.md) | Jak dát Claudovi znalosti o projektu, které si načte, jen když je potřebuje | `.claude/skills/board-tests/SKILL.md` |
| 2 | [Hooks](02-hooks.cs.md) | Jak automaticky spustit shellový příkaz, když Claude něco udělá (tady: autoformátování kódu) | `.claude/settings.json` + `.claude/hooks/format-cpp.sh` |
| 3 | [MCP](03-mcp.cs.md) | Jak Clauda připojit k externím nástrojům a zdrojům dat | `.mcp.json` |
| 4 | [Pluginy](04-plugins.cs.md) | Jak zabalit příkazy/skills/hooks, aby je mohl sdílet celý tým | `plugins/2048-dev/` + `.claude-plugin/marketplace.json` |

Každý návod existuje ve dvou jazycích: `xx-nazev.md` je anglicky,
`xx-nazev.cs.md` česky. Obsah je stejný.

## Než začneš

Potřebuješ:

1. **Nainstalovaný Claude Code** — návod na [code.claude.com](https://code.claude.com),
   ověř spuštěním `claude` v terminálu.
2. **Naklonované tohle repo** — `git clone https://github.com/klesajos/dxc-ws`
3. **Terminál otevřený ve složce repa** — všechny ukázky předpokládají, že
   pracovní adresář je kořen repa (složka s `CMakeLists.txt`).

## Rychlé demo (5 minut)

Otevři Claude Code v repu (`cd dxc-ws && claude`) a vyzkoušej:

1. **Skill** — zeptej se: *„Přidej test pro posun řádku {4, 4, 8, 0}"*.
   Sleduj, jak si Claude před napsáním testu načte skill `board-tests`.
   Můžeš ho vyvolat i ručně napsáním `/board-tests`.
2. **Hook** — požádej Clauda, ať upraví libovolný `.cpp` soubor. Hned po
   úpravě hook spustí na souboru `clang-format` — kód je vždycky
   zformátovaný, i kdyby ho Claude napsal rozházeně.
3. **MCP** — napiš `/mcp`, uvidíš dva projektové servery. Pak se zeptej:
   *„Použij deepwiki a zjisti, jak fungují generátory v Catch2."*
4. **Plugin** — napiš `/2048-dev:build-test` a jedním příkazem se projekt
   nakonfiguruje, sestaví a otestuje.

## Kdy použít který mechanismus?

Častá otázka začátečníků. Základní pravidlo:

- **Skill** — když chceš, aby Claude něco *věděl* (konvence, postupy).
  Kdy znalost použít, rozhoduje Claude. Ber to jako dokumentaci pro AI.
- **Hook** — když se něco musí stát *pokaždé*, bez výjimky (formátování,
  lint, blokování nebezpečných příkazů). Model do toho nemá co mluvit.
- **MCP** — když Claude potřebuje *schopnost*, kterou nemá (dotaz do
  databáze, hledání v dokumentaci, ovládání prohlížeče).
- **Plugin** — když chceš cokoli z výše uvedeného *sdílet* mezi více repy
  nebo s celým týmem jako jeden verzovaný balíček.
