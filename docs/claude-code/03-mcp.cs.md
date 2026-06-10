> 🌍 Číst v jazyce: [English](03-mcp.md) | **Česky**

# Ukázka 3: Projektová MCP připojení

## Co je MCP?

**MCP (Model Context Protocol)** je standard pro připojování AI asistentů
k externím nástrojům. **MCP server** je malý program, který Claudovi nabízí
schopnosti navíc — dotazy do databáze, hledání v dokumentaci, ovládání
prohlížeče, čtení ticketovacího systému.

Claude Code umí sám od sebe číst/upravovat soubory a spouštět příkazy
v terminálu. MCP je způsob, jak mu dát *všechno ostatní*.

## Co dělá tahle ukázka

Soubor `.mcp.json` v kořenu repa připojuje dva servery. Protože je
commitnutý v gitu, jsou připojení **projektová**: dostane je každý, kdo si
repo naklonuje (po jednorázovém schválení — viz bezpečnostní poznámka níže).

## Soubor řádek po řádku

```json
{
  "mcpServers": {
    "deepwiki": {
      "type": "http",
      "url": "https://mcp.deepwiki.com/mcp"
    },
    "memory": {
      "type": "stdio",
      "command": "npx",
      "args": ["-y", "@modelcontextprotocol/server-memory"]
    }
  }
}
```

- `"mcpServers"` — hlavní klíč; každá položka uvnitř je jeden server.
- `"deepwiki"` / `"memory"` — názvy, které si volíš sám. Uvidíš je v `/mcp`
  a v názvech nástrojů.

**Server `deepwiki` — transport `http` (vzdálená služba):**

- `"type": "http"` — server běží někde na internetu; Claude s ním mluví
  přes HTTPS. Na tvém stroji neběží nic.
- `"url"` — adresa serveru. DeepWiki odpovídá na otázky o dokumentaci
  libovolného veřejného GitHub repozitáře, bez registrace.

**Server `memory` — transport `stdio` (lokální program):**

- `"type": "stdio"` — Claude Code **spustí program na tvém stroji**
  a komunikuje s ním přes standardní vstup/výstup. Nejběžnější typ pro
  lokální nástroje.
- `"command": "npx"` — program ke spuštění. `npx` je součást Node.js
  a spouští npm balíček bez trvalé instalace.
- `"args": ["-y", "@modelcontextprotocol/server-memory"]` — argumenty
  příkazu, přesně jako bys v terminálu napsal
  `npx -y @modelcontextprotocol/server-memory`. `-y` přeskočí dotaz npx
  „nainstalovat balíček?". Memory server dává Claudovi malý znalostní
  graf, kam si během session ukládá fakta.

## Co se servery, které chtějí heslo nebo API klíč?

**Nikdy nepiš tajemství do `.mcp.json`** — soubor je commitnutý a klíč by
unikl všem. Místo toho odkaž na proměnnou prostředí:

```json
{
  "mcpServers": {
    "my-api": {
      "type": "http",
      "url": "https://api.example.com/mcp",
      "headers": { "Authorization": "Bearer ${MY_API_TOKEN}" }
    }
  }
}
```

`${MY_API_TOKEN}` se za běhu nahradí hodnotou té proměnné z tvého shellu —
každý vývojář si drží vlastní klíč lokálně (např. v `~/.zshrc` nebo
v `.env` souboru, který je v `.gitignore`).

## Bezpečnostní poznámka: schvalovací dialog

Projektový `.mcp.json` znamená, že *cizí konfigurace může spouštět programy
na tvém stroji* (typ `stdio` doslova startuje proces). Proto se Claude Code
při prvním otevření repa zeptá, jestli **projektové servery schvaluješ**.
Před schválením se podívej, co v souboru je — přesně to teď čtením tohohle
návodu děláš.

## Přidej vlastní server, krok za krokem

1. **Najdi server.** Projdi registr na
   [github.com/modelcontextprotocol/servers](https://github.com/modelcontextprotocol/servers)
   nebo dokumentaci dodavatele (GitHub, Sentry, Postgres... většina ho má).
2. **Přidej položku do `.mcp.json`** podle jednoho ze dvou vzorů výše
   (`http` pro hostované servery, `stdio` pro lokální).
3. **Zkontroluj validitu JSONu** — chybějící čárka je chyba č. 1:
   ```bash
   jq . .mcp.json
   ```
   Když `jq` soubor vypíše zpět, je validní; když vypíše chybu, oprav
   syntaxi.
4. **Spusť novou session Claude Code** v repu a server schval.
5. **Ověř přes `/mcp`** — příkaz vypíše každý server, jeho stav a nástroje,
   které poskytuje.
6. **Commitni soubor.**

## Vyzkoušej demo

1. Napiš `/mcp` — měl bys vidět `deepwiki` a `memory`, oba připojené.
2. Zeptej se: *„Použij deepwiki a zjisti, jak fungují generátory
   v catchorg/Catch2, pak přidej test slideLineLeft postavený na
   generátorech."* Claude zavolá externí dokumentační nástroj a odpověď
   rovnou použije v projektu.

## Když něco nefunguje

- **Server chybí v `/mcp`** → nevalidní JSON (spusť `jq . .mcp.json`),
  nebo jsi odmítl schvalovací dialog — spusť
  `claude mcp reset-project-choices` a otevři repo znovu.
- **`memory` nenastartuje** → potřebuješ nainstalovaný Node.js
  (`node --version`); první spuštění `npx` navíc potřebuje internet
  ke stažení balíčku.
- **`deepwiki` vyprší** → zkontroluj síť/proxy; server je vzdálený.
