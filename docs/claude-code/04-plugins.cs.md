> 🌍 Číst v jazyce: [English](04-plugins.md) | **Česky**

# Ukázka 4: Projektový plugin

## Co je plugin?

**Plugin** balí rozšíření Claude Code — slash příkazy, skills, agenty,
hooks, MCP servery — do jednoho instalovatelného, verzovaného balíčku.
Pluginy se šíří přes **marketplace**: katalogový soubor, který říká
„tyhle pluginy existují a tady je najdeš".

V ukázkách 1–3 jsi dal skill, hook a MCP servery přímo do repa. To skvěle
funguje pro *jeden* projekt. Plugin je další krok: stejný obsah zabalený
tak, aby šel sdílet mezi mnoha repy a týmy.

Tahle ukázka je nejmenší možná sestava: repo je **samo sobě marketplacem**
a nabízí **jeden plugin**, který balí **jeden skill** (a od Ukázky 5 i agenta).

## Z čeho se to skládá

```
.claude-plugin/marketplace.json      ← katalog marketplace (kořen repa)
plugins/2048-dev/                    ← samotný plugin
  .claude-plugin/plugin.json         ← manifest pluginu (název, verze, ...)
  skills/build-test/SKILL.md         ← jeden zabalený skill (býval příkaz)
.claude/settings.json                ← automaticky registruje + zapíná plugin
```

Tři vrstvy zevnitř ven: **skill** žije v **pluginu**, ten je uvedený
v **marketplace**, na který ukazuje projektové **nastavení**.

## Vrstva 1: skill (`plugins/2048-dev/skills/build-test/SKILL.md`)

Tenhle plugin balí **skill**. Název složky skillu = název příkazu
(`skills/build-test/` → `/2048-dev:build-test`; prefix je název pluginu).

> **Proč skill, a ne příkaz?** Vlastní slash příkazy se **sloučily do skills** —
> `commands/build-test.md` i `skills/build-test/SKILL.md` vytvoří
> `/2048-dev:build-test`. Legacy forma `commands/` dál funguje, ale skills jsou
> cesta dopředu (viz [tahák](cheatsheet.cs.md)). Tenhle plugin dodává skill.

```markdown
---
name: build-test
description: Configure, build, and run the full test suite, then summarize results
allowed-tools: Bash(cmake:*), Bash(ctest:*)
disable-model-invocation: true
---

# Build and test

Build the project and run the tests:

1. Configure if needed: `cmake -S . -B build`
...
```

- `name` — identifikátor skillu; s prefixem pluginu je to `/2048-dev:build-test`.
- `description` — zobrazí se v našeptávači příkazů.
- `allowed-tools` — předschválí nástroje, které skill potřebuje: `Bash(cmake:*)`
  znamená „jakýkoli `cmake` příkaz" smí běžet bez ptaní. Vymezuj úzce —
  *konkrétní* nástroje, ne „všechno".
- `disable-model-invocation: true` — **ten jeden flag, díky kterému se skill
  chová jako starý příkaz**: Claude ho sám nespustí, jen ty přes
  `/2048-dev:build-test`. Vynech ho a Claude může skill vyvolat i sám, když tvůj
  požadavek odpovídá `description`.
- Tělo je **prompt**, který Claude vykoná, když ho spustíš. Piš ho jako přesné
  zadání práce: očíslované kroky a co má být ve výstupu.

## Vrstva 2: manifest pluginu (`plugins/2048-dev/.claude-plugin/plugin.json`)

```json
{
  "name": "2048-dev",
  "description": "Dev workflow skill (build-test) and the game-explorer agent for the 2048 workshop project",
  "version": "1.2.0",
  "author": { "name": "Josef Klesa", "url": "https://github.com/klesajos" }
}
```

- `name` — identifikátor pluginu; stane se prefixem příkazů
  (`/2048-dev:...`). Jediné povinné pole.
- `version` — uživatelé vidí, kdy se plugin změnil, a můžou aktualizovat.
- Manifest musí ležet ve složce `.claude-plugin/` **uvnitř adresáře
  pluginu**. Složky s obsahem (`commands/`, `skills/`, `agents/`,
  `hooks/`) leží **vedle** `.claude-plugin/`, ne uvnitř — nejčastější
  začátečnická chyba.

## Vrstva 3: marketplace (`.claude-plugin/marketplace.json` v kořenu repa)

```json
{
  "name": "dxc-ws",
  "owner": { "name": "Josef Klesa" },
  "description": "Plugins for the dxc-ws Claude Code workshop",
  "plugins": [
    {
      "name": "2048-dev",
      "source": "./plugins/2048-dev",
      "description": "Dev workflow commands for the 2048 workshop project"
    }
  ]
}
```

- `name` — identifikátor marketplace; objeví se za zavináčem
  v `2048-dev@dxc-ws`.
- `owner` — kdo katalog spravuje.
- `plugins` — seznam nabízených pluginů. `source: "./plugins/2048-dev"`
  je **relativní cesta uvnitř tohohle repa** — plugin cestuje s projektem.
  Source může ukazovat i jinam (jiné GitHub repo, npm) — tak firmy
  provozují jeden centrální marketplace pro mnoho pluginů.

## Vrstva 4: automatické zapnutí (`.claude/settings.json`)

```json
{
  "extraKnownMarketplaces": {
    "dxc-ws": {
      "source": { "source": "github", "repo": "klesajos/dxc-ws" }
    }
  },
  "enabledPlugins": {
    "2048-dev@dxc-ws": true
  }
}
```

- `extraKnownMarketplaces` — „tenhle projekt používá marketplace jménem
  `dxc-ws`, stáhni ho z GitHub repa `klesajos/dxc-ws`". Komukoli, kdo
  projekt otevře, se marketplace zaregistruje automaticky (po potvrzení
  důvěry).
- `enabledPlugins` — „z toho marketplace zapni `2048-dev`". Formát klíče
  je `<nazev-pluginu>@<nazev-marketplace>`.

Dohromady: naklonuj repo → otevři Claude Code → potvrď jeden dialog →
`/2048-dev:build-test` prostě funguje.

## Postav si vlastní plugin, krok za krokem

1. **Vytvoř kostru pluginu:**
   ```bash
   mkdir -p plugins/muj-plugin/.claude-plugin plugins/muj-plugin/skills/hello
   ```
2. **Napiš manifest** `plugins/muj-plugin/.claude-plugin/plugin.json` —
   zkopíruj příklad z Vrstvy 2, změň `name` a `description`.
3. **Přidej skill** `plugins/muj-plugin/skills/hello/SKILL.md` — frontmatter
   s `name` + `description`, tělo s instrukcemi (viz Vrstva 1).
4. **Uveď ho v marketplace** — přidej položku do pole `plugins`
   v `.claude-plugin/marketplace.json` se `source: "./plugins/muj-plugin"`.
5. **Zvaliduj** — Claude Code má vestavěnou kontrolu:
   ```bash
   claude plugin validate .
   ```
   Cokoli nahlásí, oprav, než budeš pokračovat.
6. **Otestuj lokálně** uvnitř Claude Code:
   ```
   /plugin marketplace add ./
   /plugin install muj-plugin@dxc-ws
   ```
   Pak napiš `/muj-plugin:hello` a příkaz se spustí.
7. **Zapni ho pro všechny** — přidej `"muj-plugin@dxc-ws": true` do
   `enabledPlugins` v `.claude/settings.json`.
8. **Všechno commitni a pushni.**

## Plugin vs. samostatný skill/hook — co zvolit?

- **Samostatně** (`.claude/skills/`, hooks v settings): jeden projekt,
  nulová režie. Začni tady — to jsou ukázky 1 a 2.
- **Plugin**: verzovaný, sdílitelný mezi repy, balí víc kusů dohromady,
  instaluje se jedním příkazem. Na plugin přejdi, když se skill nebo hook
  osvědčí i mimo jeden projekt.

## Volitelné parametry

**V `plugin.json`** (povinné je jen `name`):

| Pole | Co dělá |
|------|---------|
| `displayName` | Lidsky čitelný název v UI pluginů (smí obsahovat mezery) |
| `version` | Sémantická verze, např. `1.2.0`; bez ní se použije git commit |
| `homepage`, `repository`, `license`, `keywords` | Metadata zobrazená ve výpisu marketplace |
| `defaultEnabled: false` | Plugin se šíří jako opt-in: uživatelé ho vidí, ale musí si ho zapnout sami |
| `commands`, `skills`, `agents`, `hooks`, `mcpServers` | Přepíšou výchozí cesty k obsahu — např. nasměruj `hooks` na vlastní `hooks/hooks.json`. Plugin může balit **všechno tohle**, nejen příkazy |

**V položce pluginu v `marketplace.json`:**

| Pole | Co dělá |
|------|---------|
| `description` | Zobrazí se ve výpisu marketplace (přebije ho plugin.json, když je v obou) |
| `version` | Verze na úrovni katalogu (plugin.json má přednost, když jsou obě) |
| `category`, `tags` | Filtrování a hledání ve větších marketplace |

Úplná reference: [reference pluginů](https://code.claude.com/docs/en/plugins-reference)
a [dokumentace marketplace](https://code.claude.com/docs/en/plugin-marketplaces).

## Kde to funguje: CLI, Desktop aplikace, Cowork

| Platforma | Funguje? | Nastavení |
|-----------|----------|-----------|
| **Claude Code CLI** (terminál) | ✅ Ano | Nic navíc — `extraKnownMarketplaces` + `enabledPlugins` v projektovém nastavení plugin zaregistrují a zapnou po jednom potvrzení důvěry |
| **Claude Desktop — záložka Code** | ✅ Ano | Stejně jako CLI: otevři projekt, potvrď dialog důvěry, `/2048-dev:build-test` se objeví |
| **Cowork** (v Desktop aplikaci) | ⚠️ Částečně | Cowork **nečte** in-repo marketplace ani projektové nastavení. Má ale vlastní systém pluginů: nainstaluj plugin přes správu pluginů v Coworku (nebo nahraj zabalený `.plugin` soubor) a jeho příkazy/skills pak v Cowork úlohách fungují. **Hooks** v pluginu závislé na lokálních nástrojích se v sandboxu můžou chovat jinak |

Ze čtyř mechanismů jsou tedy pluginy do Coworku nejpřenositelnější —
*obsah* tam funguje, nefunguje jen in-repo *distribuční kanál*
(`extraKnownMarketplaces` v projektovém nastavení). Pro tým na Coworku
publikuj plugin do sdíleného marketplace a instaluj ho přes správu
pluginů v Coworku.

## Vyzkoušej demo

1. Otevři Claude Code v čerstvém klonu → potvrď dialog důvěry marketplace.
2. Napiš `/2048-dev:` — `build-test` se našeptá.
3. Spusť ho. Claude nakonfiguruje, sestaví, pustí `ctest` a vrátí přehled
   prošlo/neprošlo, aniž by cokoli opravoval — přesně podle zadání
   v souboru příkazu.

## Když něco nefunguje

- **Příkaz se nenašeptává** → spusť `claude plugin validate .`;
  zkontroluj v `/plugin` → „Manage plugins", že je plugin zapnutý.
- **„Marketplace not trusted"** → odmítl jsi dialog; přidej ručně přes
  `/plugin marketplace add ./`.
- **Změnil jsi soubor skillu a nic se neděje** → spusť novou session;
  obsah pluginů se načítá při startu session.
