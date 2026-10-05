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

Tenhle průvodce projde formátovací hook celý, pak přidá **druhý hook** na jinou
událost — `Stop` hook, který spustí testy, když Claude dokončí odpověď — pak dva
**hooky pro pojistku a kontext** (`PreToolUse`, `UserPromptSubmit`) a vysvětlí
tři **typy** hooků.

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

## Druhý hook: brána na testy (událost `Stop`)

Formátovací hook reaguje na **nástroj** (`Edit`/`Write`). Hooks ale umí
reagovat i na **životní cyklus session**. Tohle repo má druhý hook na události
`Stop` — která nastane jednou, když Claude dokončí svou odpověď — aby spustil
testovací sadu a oznámil, jestli je strom stále zelený.

Jeho registrace sedí hned vedle prvního hooku v `.claude/settings.json`. Všimni
si, že tu **není `matcher`**: `Stop` není o nástroji, takže není co filtrovat.

```json
"Stop": [
  {
    "hooks": [
      { "type": "command",
        "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/run-tests.sh" }
    ]
  }
]
```

Skript (`.claude/hooks/run-tests.sh`) je záměrně **informativní**: vyprázdní
JSON události Stop, spustí `ctest`, vypíše jeden řádek a vždycky `exit 0` — takže
tě nikdy nepřeruší. Jeho výkonné řádky, doslova ze souboru:

```bash
cat >/dev/null
project_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}"
build_dir="$project_dir/build"
if [ ! -d "$build_dir" ] || ! command -v ctest >/dev/null 2>&1; then
    exit 0
fi
summary=$(ctest --test-dir "$build_dir" 2>/dev/null | grep -E 'tests passed' | tail -1 || true)
if [ -z "$summary" ]; then
    exit 0
fi
case "$summary" in
    "100% tests passed"*) echo "run-tests hook: ✓ $summary" ;;
    *) echo "run-tests hook: ✗ $summary (run 'ctest --test-dir build' for details)" ;;
esac
```

- **Stráž (guard)** `if [ ! -d "$build_dir" ] || ! command -v ctest …; then exit 0; fi`
  skončí, když neexistuje adresář `build/` **nebo** není `ctest` v PATH — takže
  na čerstvém, nesestaveném klonu zůstane potichu místo toho, aby chybovala.
- **Řádek se souhrnem** `ctest … | grep -E 'tests passed' | tail -1` ponechá jen
  jednořádkový součet ctestu (např. `100% tests passed out of 13` u ctest 4.x);
  `|| true` zabrání tomu, aby neúspěšný běh shodil skript pod `set -e`, a následné
  `[ -z "$summary" ]` tiše skončí, když build zatím žádné testy neregistruje.
- **Verdikt** — `case` vypíše `run-tests hook: ✓ $summary`, když součet začíná
  `100% tests passed`, jinak `run-tests hook: ✗ $summary` s tipem, ať znovu spustíš
  `ctest`.

**Chceš, aby spíš *blokoval* než informoval?** `Stop` hook, který skončí
nenulovým kódem (nebo vypíše `{"decision": "block", "reason": "..."}` na stdout),
říká Claudovi, že **ještě není** hotovo — tak vynutíš „testy musí projít, než
skončíš". My ten náš necháváme informativní, aby se živá workshopová session
nezasekla v opravovací smyčce; přepni ho na blokující, když chceš tvrdou bránu.

## Vyzkoušej druhý hook

Nejdřív projekt jednou sestav (`cmake --build build`), pak se Clauda na cokoli
zeptej. Když odpověď skončí, `Stop` hook spustí testovací sadu a v přepisu se
objeví informativní řádek:

```
run-tests hook: ✓ 100% tests passed out of 13
```

Než poprvé sestavíš, hook zůstává potichu — to dělá svou práci ta stráž výše.

## Další dva hooky: pojistka a živý git kontext

Repo obsahuje ještě dva `command` hooky. Spolu s prvními dvěma pokrývají čtyři
události, které budeš používat nejčastěji: `UserPromptSubmit` → `PreToolUse` →
`PostToolUse` → `Stop`.

Jejich registrace, doslova z `.claude/settings.json`:

```json
    "UserPromptSubmit": [
      {
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/git-context.sh"
          }
        ]
      }
    ],
    "PreToolUse": [
      {
        "matcher": "Bash",
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/block-destructive.sh"
          }
        ]
      }
    ],
```

### Pojistka: `block-destructive.sh` (`PreToolUse`)

Spustí se **před** každým Bash příkazem a odmítne ty, které zničí práci, jakou
ti git už nevrátí: rekurzivní vynucené mazání (kromě `rm -rf build`),
`git reset --hard`, force-push (`--force-with-lease` je povolený),
`git clean -f` a `git checkout -- .` / `git restore .`.

```bash
#!/usr/bin/env bash
# PreToolUse hook (matcher: Bash): refuse shell commands that destroy work
# which git can't give back. Claude receives the reason and picks a safer
# route; everything else passes through untouched.
set -euo pipefail

input=$(cat)

if command -v jq >/dev/null 2>&1; then
    command=$(printf '%s' "$input" | jq -r '.tool_input.command // empty')
else
    command=$(printf '%s' "$input" | python3 -c \
        'import json,sys; print(json.load(sys.stdin).get("tool_input", {}).get("command", ""))')
fi

deny() {
    printf '{"hookSpecificOutput":{"hookEventName":"PreToolUse","permissionDecision":"deny","permissionDecisionReason":"%s"}}\n' "$1"
    exit 0
}

# Wiping the build directory is routine; any other recursive force-delete is not.
# Flags are checked separately so `rm -rf`, `rm -fr` and `rm -r -f` all match.
if printf '%s' "$command" | grep -Eq '(^|[;&|[:space:]])rm[[:space:]]' \
    && printf '%s' "$command" | grep -Eq '[[:space:]](-[[:alpha:]]*[rR]|--recursive)' \
    && printf '%s' "$command" | grep -Eq '[[:space:]](-[[:alpha:]]*f|--force)' \
    && ! printf '%s' "$command" | grep -Eq '^rm -rf (\./)?build/?$'; then
    deny "block-destructive hook: recursive force-delete is blocked. Only 'rm -rf build' is allowed; delete specific files instead."
fi

if printf '%s' "$command" | grep -Eq 'git[[:space:]]+reset[[:space:]].*--hard'; then
    deny "block-destructive hook: git reset --hard discards uncommitted work. Use git stash, or git reset --soft."
fi

# --force-with-lease is the safe variant, so only bare -f / --force is blocked.
if printf '%s' "$command" | grep -Eq 'git[[:space:]]+push([[:space:]].*)?[[:space:]](-f|--force)([[:space:]]|$)'; then
    deny "block-destructive hook: force-push is blocked. Use git push --force-with-lease, or ask the user."
fi

if printf '%s' "$command" | grep -Eq 'git[[:space:]]+clean[[:space:]]+-[[:alpha:]]*f'; then
    deny "block-destructive hook: git clean -f deletes untracked files for good. Run git clean -n first and show the user the list."
fi

if printf '%s' "$command" | grep -Eq 'git[[:space:]]+(checkout[[:space:]]+(--[[:space:]]+)?|restore[[:space:]]+)\.([[:space:]]|$)'; then
    deny "block-destructive hook: this discards every uncommitted change. Restore individual files, or git stash."
fi

exit 0
```

- **Jak blokuje:** vypíše JSON objekt s `"permissionDecision": "deny"` uvnitř
  `hookSpecificOutput` a skončí s kódem 0. `permissionDecisionReason` jde
  **Claudovi**, takže zpráva mu říká, co udělat místo toho („použij
  `--force-with-lease`"). Blokuje i návratový kód 2 s důvodem na stderr.
- **Ticho = povolení.** Žádný výstup a exit 0 znamená „nemám názor": rozhodnou
  běžná pravidla oprávnění a aktuální režim.
- **Porovnává text, takže počítej s falešnými poplachy.** Při psaní tohoto
  průvodce hook zablokoval příkaz, který slova `git reset --hard` jen *obsahoval*
  uvnitř heredocu. Hooky na vzory se raději mýlí ve prospěch blokování. Pro
  pojistku je to v pořádku, ale řekni lidem, jak to obejít (tady: dej text do
  souboru).
- **Hook vs. deny pravidlo:** deny pravidlo `Bash(git push --force *)`
  v `settings.json` ([Ukázka 9](09-permissions-sandbox.cs.md)) je jednodušší, ale
  neumí výjimky (`rm -rf build` ano, `rm -rf src` ne) ani se vysvětlit. Obojí
  porovnává **text příkazu**, takže ani jedno není bezpečnostní hranice:
  `bash -c '…'` obejde obojí. Skutečnou hranici dá sandbox.

### Živý kontext: `git-context.sh` (`UserPromptSubmit`)

Spustí se, když odešleš prompt, **dřív**, než ho Claude uvidí, a přidá jeden řádek
jako `Git: on branch main, 2 uncommitted file(s): src/board.cpp, tests/test_board.cpp`.

```bash
#!/usr/bin/env bash
# UserPromptSubmit hook: before Claude sees each prompt, tell it which branch
# it is on and which files are uncommitted. Saves Claude a `git status` call
# and stops it from assuming a clean tree.
set -euo pipefail

# Drain the prompt JSON on stdin; this hook doesn't need any of its fields.
cat >/dev/null

project_dir="${CLAUDE_PROJECT_DIR:-$(pwd)}"
cd "$project_dir"

# Outside a git checkout there's nothing to report.
git rev-parse --is-inside-work-tree >/dev/null 2>&1 || exit 0

branch=$(git branch --show-current 2>/dev/null)
branch=${branch:-"(detached HEAD)"}
changes=$(git status --porcelain 2>/dev/null)

if [ -z "$changes" ]; then
    context="Git: on branch $branch, working tree clean."
else
    count=$(printf '%s\n' "$changes" | wc -l | tr -d ' ')
    # Cap the list so a huge diff can't flood the context window.
    files=$(printf '%s\n' "$changes" | head -10 | awk '{print $NF}' | paste -sd ',' - | sed 's/,/, /g')
    context="Git: on branch $branch, $count uncommitted file(s): $files"
    [ "$count" -gt 10 ] && context="$context, ..."
fi

# json.dumps escapes quotes and backslashes in file names.
CONTEXT="$context" python3 -c '
import json, os
print(json.dumps({"hookSpecificOutput": {
    "hookEventName": "UserPromptSubmit",
    "additionalContext": os.environ["CONTEXT"]}}))'
```

- **Jak přidává kontext:** JSON s `hookSpecificOutput.additionalContext`.
  Claude Code zabalí řetězec do systémové připomínky (system reminder), kterou
  Claude čte, ale v chatu se jako zpráva neobjeví. Funguje i obyčejný stdout;
  JSON je bezpečnější, protože `json.dumps` escapuje podivné názvy souborů.
- **Drž ho malý:** běží u **každého** promptu, takže pokaždé stojí tokeny.
  Seznam je omezený na 10 souborů a dokumentace omezuje `additionalContext`
  jakéhokoli hooku na 10 000 znaků.
- **Musí být rychlý:** prompt na hook čeká. `git status` trvá milisekundy;
  síťové volání by tady zpomalilo každý prompt.

## Vyzkoušej dva nové hooky

1. Požádej Clauda: *„Spusť přesně tenhle příkaz: git push --force nowhere main"*.
   Hook ho zablokuje dřív, než git poběží. Claude ocituje
   `block-destructive hook: force-push is blocked…` a navrhne
   `--force-with-lease`. (Remote `nowhere` neexistuje, takže test je neškodný
   i bez hooku.)
2. Uprav libovolný soubor a pak se zeptej: *„Bez spuštění jakéhokoli nástroje: na
   které jsem větvi a co není commitnuté?"* Claude odpoví z vloženého kontextu.

Hook otestuješ i bez Clauda tak, že do něj pošleš falešnou událost:

```bash
echo '{"tool_input":{"command":"git push -f"}}' | .claude/hooks/block-destructive.sh
```

## Typy hooků: command, prompt, agent

Oba hooky výše používají `"type": "command"` — shellový skript. To je
nejběžnější typ, ale ne jediný. Handler může být jeden ze tří typů a směňuje
determinismus za úsudek:

| Typ | Co běží | Na co se hodí |
|-----|---------|---------------|
| `command` | Shellový skript (naše dva hooky) | Deterministické, rychlé kontroly — formát, lint, spuštění testů, zablokování zakázaného příkazu |
| `prompt` | Jednokolové volání malého modelu Claude | Rychlý úsudek — „je commit message výstižná?" — vrací rozhodnutí ano/ne |
| `agent` | Vícekolový subagent s přístupem k nástrojům (`Read`, `Grep`, …) | Hloubková kontrola — „přečti diff a ověř, že nepřibyla žádná tajemství" — než se pokračuje |

`prompt` hook se registruje s textem k vyhodnocení místo příkazu:

```json
{ "type": "prompt",
  "prompt": "Přidává staged diff test ke každé nové veřejné metodě? Odpověz ano nebo ne." }
```

Základní pravidlo: sáhni nejdřív po `command` (je zdarma a okamžitý), po `prompt`,
když kontrola potřebuje porozumění jazyku, a po `agent` jen tehdy, když se
kontrola sama musí prohrabat kódem. Tohle repo dodává `command` hooky; o dalších
dvou stačí vědět, že existují.

## Volitelné parametry

Hook handler umí víc než `type` a `command`. Nejužitečnější volitelná pole:

| Pole | Co dělá |
|------|---------|
| `timeout` | Sekundy do zrušení hooku (výchozí 600). U rychlých hooků nastav nízko, aby zaseknutý skript nezdržoval session |
| `statusMessage` | Vlastní text u spinneru během běhu, např. `"Formátuji C++..."` |
| `if` | Dodatečný filtr syntaxí permission pravidel, např. `"if": "Edit(*.cpp)"` — přesnější než `matcher`, který vidí jen název nástroje |
| `once: true` | Spustit jen jednou za session, pak odregistrovat (hodí se pro kontroly prostředí) |

A události: tahle ukázka používá `PostToolUse`, ale hooks se dají navěsit
na celý životní cyklus session. Které stojí za to znát nejdřív:

| Událost | Spustí se |
|---------|-----------|
| `PreToolUse` | Před spuštěním nástroje — **umí ho zablokovat** (např. zakázat `git push --force`) |
| `PostToolUse` | Po úspěšném nástroji (náš případ) |
| `SessionStart` | Při startu session — kontroly prostředí, načtení kontextu |
| `UserPromptSubmit` | Při odeslání promptu — umí přidat kontext navíc |
| `Stop` | Když Claude končí odpověď — např. ověřit, že testy opravdu běžely |

Úplný seznam událostí a polí: [oficiální dokumentace hooks](https://code.claude.com/docs/en/hooks).

## Další recepty na hooky

Nápady, které stojí za to znát, ale v repu nejsou — buď proto, že jsou osobní
(notifikace), potřebují infrastrukturu (auditní endpoint), nebo se nehodí ke
C++ hře. Každá událost níže v Claude Code 2.1.283 existuje:

| Recept | Událost | Co dělá |
|--------|---------|---------|
| Spustit jen relevantní testy | `PostToolBatch` | Po dokončení dávky paralelních úprav spustí testy jen pro dotčené soubory, jednou za dávku místo jednou za úpravu |
| Desktopová notifikace | `Notification` | `osascript -e 'display notification "Claude needs you"'`, když Claude čeká na oprávnění (`permission_prompt`) nebo na vstup (`idle_prompt`). Patří do tvého **osobního** `~/.claude/settings.json` |
| Ochrana souborů | `PreToolUse` (`Edit\|Write`) | Zakáže úpravy lockfilů, `.git/` a generovaného kódu. Na prosté blokování cest je jednodušší deny pravidlo `Edit(...)` ([Ukázka 9](09-permissions-sandbox.cs.md)) |
| Sken tajemství | `PreToolUse` / `Stop` | Odmítne úpravy obsahující klíče, nebo před dokončením spustí `gitleaks` na diff |
| Brána před commitem | `PreToolUse` (`Bash`, `"if": "Bash(git commit *)"`) | Před `git commit` spustí lint/testy nebo zkontroluje formát commit message |
| AI review brána | `Stop`, `"type": "prompt"` | Malý model zkontroluje „splňuje změna zadání?" a když ne, zablokuje ukončení. Stojí jedno volání modelu za odpověď |
| Načtení dev prostředí | `SessionStart` + `CwdChanged` | Spustí `direnv` nebo podobný nástroj při startu session nebo když Claude udělá `cd` |
| Znovu vložit omezení | `PostCompact` | Po kompakci kontextu připomene Claudovi aktivní ticket nebo akceptační kritéria |
| Hlídání změn konfigurace | `ConfigChange` | Loguje nebo blokuje úpravy nastavení, pravidel či skillů během session. Hodí se v regulovaném prostředí |
| Auditní stopa | `PostToolUse`, `"type": "http"` | Pošle (POST) každé volání nástroje na interní endpoint |
| Cenzura obrazovky | `MessageDisplay` | Odstraní z vykreslovaného výstupu názvy hostů nebo ID zákazníků, aniž by měnil přepis konverzace |

## Kde to funguje: CLI, Desktop aplikace, Cowork

| Platforma | Funguje? | Nastavení |
|-----------|----------|-----------|
| **Claude Code CLI** (terminál) | ✅ Ano | Nic navíc — hooks z `.claude/settings.json` se načtou při startu session |
| **Claude Desktop — záložka Code** | ✅ Ano | Stejný engine, stejné konfigurační soubory jako CLI. Potvrď jednorázový dialog důvěry projektu; hooks pak běží stejně |
| **Cowork** (v Desktop aplikaci) | ❌ Ne | Sandboxované VM Coworku projektové hooks z `.claude/settings.json` nespouští. Přímá náhrada není — nejblíž jsou hooks zabalené v nainstalovaném pluginu |

Poznámka pro workshop: tohle je nejvýraznější platformní rozdíl ze všech
šesti ukázek. Hooks jsou *lokální automatizace* — pokud na nich tvůj workflow
stojí (formátování, lint brány), pracuj v CLI nebo v záložce Code v Desktopu,
ne v Coworku.

## Když něco nefunguje

- **Hook se nikdy nespustí** → po úpravě `settings.json` je potřeba nová
  session; zkontroluj taky spustitelnost skriptu (`ls -l .claude/hooks/`).
- **„Permission denied"** → přeskočil jsi `chmod +x`.
- **Chyby hooku ruší práci** → každá větev „není co dělat" musí končit
  `exit 0`, ne chybou.
