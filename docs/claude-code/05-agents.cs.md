> 🌍 Číst v jazyce: [English](05-agents.md) | **Česky**

# Ukázka 5: Subagenti (vlastní agenti)

## Co je subagent?

**Subagent** je specializovaný pomocník, který běží ve **vlastním izolovaném
kontextu** s **vlastními nástroji a personou**. Když mu hlavní konverzace předá
úkol, subagent ho zpracuje zvlášť a vrátí výsledek — jeho mezikroky ti
nezahltí hlavní kontext.

Subagenta od dřívějších ukázek odlišují dvě věci:

- **Izolace kontextu.** Skill (Ukázka 1) načte instrukce *do tvé konverzace*.
  Subagent běží v *oddělené* konverzaci. Hodí se, když je úkol samostatný
  („zkontroluj tenhle diff", „napiš tenhle test") a ty chceš zpátky jen závěr.
- **Omezení nástrojů / persona.** Subagentovi můžeš dát *míň* nástrojů, než máš
  sám. Recenzent bez `Edit`/`Write` **fyzicky nemůže** měnit soubory — omezení
  je vynucené, ne jen doporučené.

Základní pravidlo: **skill** učí Clauda, *jak* něco udělat v aktuálním kontextu;
**subagent** je *ten, komu* deleguješ celý úkol.

## Co tahle ukázka dělá

Tahle ukázka přidává **tři** subagenty, kteří záměrně pokrývají celé spektrum:

| Agent | Kde žije | Nástroje | Proč tu je |
|-------|----------|----------|------------|
| `board-test-writer` | `.claude/agents/` (projekt) | čtení **+ zápis + Bash** | Umí zapisovat; **přednačítá skill `board-tests`** — ukazuje, že subagenti skládají s Ukázkou 1 |
| `cpp-reviewer` | `.claude/agents/` (projekt) | jen pro čtení (bez `Edit`/`Write`) | Učební artefakt omezení nástrojů: recenzent, který nemůže měnit kód |
| `game-explorer` | `plugins/2048-dev/agents/` (plugin) | jen pro čtení, bez `Bash` | Zabalený v pluginu z Ukázky 4; auto-objevený a pojmenovaný `2048-dev:game-explorer` |

Dohromady ukazují stejný mechanismus ve třech bodech: projektový a se zápisem,
projektový a zamčený, a zabalený v pluginu.

## Soubory řádek po řádku

Projektový subagent je jediný Markdown soubor v `.claude/agents/<jmeno>.md`:

```
.claude/              ← projektová konfigurace Claude Code (verzovaná v gitu)
└── agents/           ← tady žijí všichni projektoví subagenti
    ├── board-test-writer.md
    └── cpp-reviewer.md
```

Nejlíp se čte `cpp-reviewer` — omezení nástrojů *je* tou lekcí:

```yaml
---
name: cpp-reviewer
description: |
  Use to review C++ changes in this repo for correctness and house style...
  <example>
  user: "Review my uncommitted changes."
  assistant: "I'll run the cpp-reviewer agent — it reads the diff and reports..."
  <commentary>Read-only semantic review is exactly this agent's role...</commentary>
  </example>
tools: Read, Grep, Glob, Bash
model: inherit
color: blue
---

# C++ reviewer (read-only)
You review C++ changes... You **do not have Edit or Write tools** — that is
deliberate...
```

Co která část dělá:

- `name` — identifikátor agenta. Oslovíš ho přes `@agent-cpp-reviewer`, prostou
  prosbou („zkontroluj moje změny"), nebo `claude --agent cpp-reviewer`.
- `description` — **nejdůležitější pole.** Podle něj se Claude rozhoduje, *kdy
  agentovi delegovat sám od sebe*. Napiš ho jako spouštěč a přidej bloky
  `<example>` / `<commentary>`: každý příklad je miniscénář učící automatické
  delegování.
- `tools` — **seznam povolených nástrojů.** `Read, Grep, Glob, Bash` a **žádný
  `Edit`/`Write`** znamená, že agent může číst repo a spustit `git diff`, ale
  nemůže změnit ani jeden soubor. Vynech pole úplně = zdědí *všechny* nástroje;
  vypiš ho = zamkneš agenta. (Lze taky odečítat přes `disallowedTools`.)
- `model: inherit` — stejný model jako konverzace, která agenta spustila.
- `color` — barva v UI agentů.
- Všechno **pod** frontmatterem je **systémový prompt** agenta — jeho persona
  a instrukce, které platí jen v jeho vlastním kontextu.

Zbylí dva přidávají po jedné myšlence:

- `board-test-writer.md` má ve frontmatteru **`skills: board-tests`**. To
  **přednačte skill z Ukázky 1** do agenta, takže zdědí testovací konvence, aniž
  by je kopíroval. Ukázka skládání vrstvy na vrstvu. Navíc má `Edit, Write, Bash`,
  takže opravdu může přidat `TEST_CASE` a spustit `ctest`.
- `plugins/2048-dev/agents/game-explorer.md` žije **uvnitř pluginu**, ve složce
  `agents/` **vedle** `commands/` (viz Ukázka 4, která už `agents/` uvádí jako
  platnou složku pluginu). Claude Code ho auto-objeví a pojmenuje
  **`2048-dev:game-explorer`**. Žádný záznam v settings není potřeba.

> **Skutečná hranice schopností:** agent zabalený v pluginu **nemůže** definovat
> vlastní `hooks`, `mcpServers` ani `permissionMode` — ty patří pluginu, ne
> agentovi uvnitř. Projektoví agenti v `.claude/agents/` a pluginoví agenti
> sdílejí stejný formát souboru, ale pluginový kontext je ten omezenější.

## Vytvoř si vlastního subagenta krok za krokem

1. **Vytvoř složku** (projektovou):
   ```bash
   mkdir -p .claude/agents
   ```
2. **Vytvoř soubor** `.claude/agents/my-agent.md`:
   ```markdown
   ---
   name: my-agent
   description: Use when <situace, která má spustit delegování>.
   tools: Read, Grep, Glob
   model: inherit
   ---

   # My agent
   <Systémový prompt: kdo agent je, co dělá, jak hlásí výsledek.>
   ```
3. **Rozhodni o jeho pravomocech.** Jen pro čtení? Vypiš `Read, Grep, Glob`
   a dost. Potřebuje upravovat a sestavovat? Přidej `Edit, Write, Bash`. Pole
   `tools` vynech, jen když opravdu potřebuje všechno.
4. **Restartuj Claude Code** (nebo spusť novou session) — agenti se objevují při
   startu session.
5. **Vyvolej ho** třemi způsoby: prostou prosbou odpovídající `description`,
   zmínkou `@agent-my-agent`, nebo `claude --agent my-agent`.
6. **Zacommituj ho**, ať ho má každý, kdo si repo naklonuje:
   ```bash
   git add .claude/agents/my-agent.md && git commit -m "Add my-agent subagent"
   ```
   (Osobní agenty, které nechceš sdílet, dej do `~/.claude/agents/`.)

## Vyzkoušej demo

Spusť v čerstvé session `claude` v kořeni repa:

1. **`cpp-reviewer` (jen pro čtení).** Udělej drobnou úpravu libovolného `.cpp`
   a zeptej se: *„Zkontroluj moje necommitnuté změny."* Dostaneš nálezy
   označené závažností s `file:line` a patičkou *„No files modified"* — a
   `git status` se nezmění, protože agent nemá jak zapisovat. (Bonus: uprav
   `src/game.cpp` a měl by označit ignorovaný příznak `changed`.)
2. **`board-test-writer` (se zápisem).** Zeptej se: *„Přidej test pro řádek
   {4, 4, 8, 0}."* Načte konvence `board-tests`, přidá `TEST_CASE` a spustí
   `ctest`. Pak si vyžádej test na game-over plné desky — ověř, že **odhalí bug
   v `isGameOver()`** místo toho, aby oslabil tvrzení a vynutil průchod.
3. **`game-explorer` (zabalený v pluginu).** Zeptej se: *„Vystopuj, jak se ze
   stisku klávesy stane tah."* Dostaneš číslovanou stopu `file:line` a žádné
   úpravy. Přišel z pluginu — ověř přes `claude plugin validate .`.

## Volitelné parametry frontmatteru

| Pole | Co dělá |
|------|---------|
| `tools` | Seznam povolených nástrojů. Vynech = zdědí vše; vypiš = zamkne (např. jen pro čtení vynecháním `Edit`/`Write`) |
| `disallowedTools` | Odečte konkrétní nástroje z výchozí sady. Ignoruje se, když je nastaven `tools` |
| `model` | Model pro agenta; `inherit` = stejný jako spouštějící konverzace |
| `color` | Barva v UI agentů |
| `skills` | Přednačte jeden či víc skillů do agenta (seznam přes čárku nebo YAML pole), např. `skills: board-tests` |
| `<example>` / `<commentary>` v `description` | Scénáře, které učí Clauda, *kdy* agentovi automaticky delegovat |

Úplný aktuální seznam: [oficiální dokumentace subagentů](https://code.claude.com/docs/en/sub-agents).

## Kde to funguje: CLI, Desktop, Cowork

| Platforma | Funguje? | Nastavení |
|-----------|----------|-----------|
| **Claude Code CLI** (terminál) | ✅ Ano | Agenti v `.claude/agents/` se objeví automaticky při spuštění `claude` v repu |
| **Desktop — záložka Code** | ✅ Ano | Stejný engine jako CLI; projektové `.claude/agents/` se načtou po jednorázovém dialogu důvěry |
| **Cowork** (v Desktop appce) | ⚠️ Přes plugin | Cowork projektové `.claude/agents/` **nenačítá**. Ale agent **zabalený v pluginu** (jako `game-explorer`) jede s pluginem, když ho v Coworku nainstaluješ — takže pro přenositelnost zabal agenta do pluginu |

Je to stejný vzorec jako u Ukázky 1: projektová `.claude/` konfigurace do Coworku
nedosáhne, ale obsah zabalený v pluginu ano. `game-explorer` je přenositelný
právě proto, že žije v pluginu, ne v `.claude/agents/`.

## Řešení problémů

- **Agentovi se nikdy automaticky nedeleguje** → `description` je moc vágní nebo
  bez `<example>` bloků. Popiš *situaci uživatele* a přidej spouštěcí scénáře.
- **Agent upravil soubor, který neměl** → zdědil všechny nástroje, protože jsi
  vynechal `tools`. Přidej explicitní seznam bez `Edit`/`Write`.
- **`@agent-name` se nenašel / pluginový agent chybí** → restartuj session
  (agenti se načítají při startu); u pluginového ověř, že je zapnutý, a spusť
  `claude plugin validate .`.

---

Dál: subagent deleguje **jeden** úkol do **jednoho** izolovaného kontextu. Když
chceš orchestrovat **víc** agentů s deterministickým, kódem definovaným tokem
řízení, sáhneš po workflow → [Ukázka 6: Workflows](06-workflows.cs.md).
