> 🌍 Číst v jazyce: [English](02-hooks.md) | **Česky**

# Ukázka 2: Projektový hook

## Co je hook?

**Hook** je shellový příkaz, který Claude Code spustí **automaticky**, když
nastane určitá událost — třeba „po tom, co Claude upraví soubor" nebo „před
tím, než Claude spustí příkaz v terminálu".

Klíčový rozdíl oproti skillu: skill je *rada*, kterou se model může řídit;
hook je *vynucení*, které běží mimo model, **úplně pokaždé**. Hooks používej
na věci, které se nesmí nikdy přeskočit: formátování, lint, blokování
nebezpečných příkazů.

## Co dělá tahle ukázka

Pokaždé, když Claude upraví nebo vytvoří soubor, náš hook zkontroluje,
jestli jde o C++ soubor (`.cpp` / `.hpp`) — a pokud ano, spustí na něm
`clang-format`. Výsledek: Claudův kód je vždycky zformátovaný podle
projektových pravidel v `.clang-format`, i kdyby ho model napsal rozházeně.

Jsou v tom dva soubory:

```
.claude/settings.json        ← říká, KDY se hook spustí
.claude/hooks/format-cpp.sh  ← skript, který říká, CO se má udělat
```

## Část 1: registrace (`.claude/settings.json`) řádek po řádku

```json
{
  "hooks": {
    "PostToolUse": [
      {
        "matcher": "Edit|Write",
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/format-cpp.sh"
          }
        ]
      }
    ]
  }
}
```

- `"hooks"` — sekce nastavení, kde žijí všechny hooks.
- `"PostToolUse"` — **událost**: spustit *po* úspěšném použití nástroje.
  Další užitečné události: `PreToolUse` (před spuštěním nástroje — umí ho
  zablokovat), `SessionStart`, `UserPromptSubmit`, `Stop` (když Claude
  končí odpověď).
- `"matcher": "Edit|Write"` — filtr na **název nástroje**. Svislítko `|`
  znamená „nebo", jako v regulárních výrazech: spustit jen když byl nástroj
  `Edit` nebo `Write` (dva nástroje, kterými Claude mění soubory). Bez
  matcheru by hook běžel i po každém `Read`, `Bash` atd.
- `"type": "command"` — tenhle hook spouští shellový příkaz (existují
  i jiné typy, např. volání HTTP endpointu).
- `"command": "${CLAUDE_PROJECT_DIR}/..."` — skript, který se má spustit.
  `${CLAUDE_PROJECT_DIR}` je proměnná, kterou Claude Code nahradí
  absolutní cestou ke kořenu repa — hook tak funguje bez ohledu na to,
  ve kterém podadresáři Claude zrovna je.

## Část 2: skript (`.claude/hooks/format-cpp.sh`) řádek po řádku

```bash
#!/usr/bin/env bash
set -euo pipefail

input=$(cat)
```

- `#!/usr/bin/env bash` — tzv. „shebang": říká systému, že soubor má
  spustit bashem.
- `set -euo pipefail` — bezpečnostní pojistky: zastavit při jakékoli chybě
  (`-e`), nenastavené proměnné brát jako chybu (`-u`), selhat, když selže
  kterýkoli krok pipeline (`pipefail`). Standard pro každý bash skript.
- `input=$(cat)` — **takhle hook dostává data.** Claude Code posílá detaily
  volání nástroje jako JSON na **stdin** (standardní vstup). `cat` ho celý
  přečte a my ho uložíme do proměnné `input`.

```bash
if command -v jq >/dev/null 2>&1; then
    file_path=$(printf '%s' "$input" | jq -r '.tool_input.file_path // empty')
else
    file_path=$(printf '%s' "$input" | python3 -c \
        'import json,sys; print(json.load(sys.stdin).get("tool_input", {}).get("file_path", ""))')
fi
```

- `command -v jq` — zkontroluje, jestli je nainstalovaný JSON nástroj `jq`
  (`>/dev/null 2>&1` jen skryje výstup kontroly).
- `jq -r '.tool_input.file_path // empty'` — vytáhne z JSONu cestu
  k upravenému souboru. Vstup vypadá takto:
  `{"tool_name": "Edit", "tool_input": {"file_path": "/cesta/k/board.cpp", ...}}`,
  takže `.tool_input.file_path` doklikne k cestě. `// empty` znamená
  „když chybí, nevypisuj nic (místo slova null)".
- Větev `else` dělá totéž v Pythonu — záloha pro stroje bez `jq`.

```bash
if command -v clang-format >/dev/null 2>&1; then
    formatter="clang-format"
elif command -v xcrun >/dev/null 2>&1 && xcrun --find clang-format >/dev/null 2>&1; then
    formatter="xcrun clang-format"
else
    exit 0
fi
```

- Hledá použitelný formátovač. Na macOS `clang-format` často není v PATH,
  ale je součástí Xcode command-line tools, dostupný přes `xcrun`.
- `exit 0` — když formátovač není vůbec, skonči **úspěšně** a nedělej nic.
  Nenulový návratový kód by se Claudovi zobrazil jako chyba;
  „nenainstalovaný formátovač" nemá rozbít session.

```bash
case "$file_path" in
    *.cpp|*.hpp)
        if [ -f "$file_path" ]; then
            $formatter -i "$file_path"
            echo "format-cpp hook: formatted ${file_path##*/}"
        fi
        ;;
esac

exit 0
```

- `case ... in *.cpp|*.hpp)` — porovnání přípony souboru. Dál projdou jen
  C++ zdrojáky a hlavičky; `.md` nebo `.json` propadne a skript prostě
  skončí.
- `[ -f "$file_path" ]` — „existuje ten soubor?" (mezitím mohl být smazán).
- `$formatter -i "$file_path"` — samotná práce: `-i` znamená „in place",
  tedy přepiš soubor zformátovaným obsahem.
- `echo ...` — cokoli hook vypíše, se ukáže v přepisu konverzace
  v Claude Code, takže vidíš, že hook odvedl svou práci.
  (`${file_path##*/}` odřízne adresářovou část, zůstane jen název souboru.)

## Vytvoř si vlastní hook, krok za krokem

1. **Napiš skript** do `.claude/hooks/`, např. `muj-hook.sh`. Vyjdi
   z kostry výše: přečti stdin, vytáhni co potřebuješ, proveď akci, `exit 0`.
2. **Udělej ho spustitelným** — krok, na který každý zapomene:
   ```bash
   chmod +x .claude/hooks/muj-hook.sh
   ```
3. **Zaregistruj ho** v `.claude/settings.json` pod správnou událost +
   matcher (viz Část 1).
4. **Nejdřív ho otestuj ručně** — neladit uvnitř Clauda. Stdin JSON si
   nasimuluj sám:
   ```bash
   echo '{"tool_input":{"file_path":"src/board.cpp"}}' | .claude/hooks/muj-hook.sh
   ```
5. **Spusť novou session Claude Code** a vyvolej událost doopravdy.
6. **Commitni oba soubory.**

## Vyzkoušej demo

Požádej Clauda, ať přidá metodu do `src/board.cpp` a formátováním se
nezabývá. Po úpravě spusť `git diff` — kód už je zformátovaný a v přepisu
se objeví řádek `format-cpp hook: formatted board.cpp`.

## Když něco nefunguje

- **Hook se nikdy nespustí** → po úpravě `settings.json` je potřeba nová
  session; zkontroluj taky spustitelnost skriptu (`ls -l .claude/hooks/`).
- **„Permission denied"** → přeskočil jsi `chmod +x`.
- **Chyby hooku ruší práci** → každá větev „není co dělat" musí končit
  `exit 0`, ne chybou.
