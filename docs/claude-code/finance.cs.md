> 🌍 Číst v jazyce: [English](finance.md) | **Česky**

# Claude pro práci s financemi

Anthropic prodává řešení **„Claude for Financial Services"** určené bankám,
správcům aktiv a PE fondům. Tento návod vysvětluje, co v něm je, které části
stojí peníze navíc a jak používat ty, které nestojí — pluginy, skills
a bezplatné zdroje dat — v Claude Code nebo Coworku.

Navazuje na dvě dřívější ukázky: [MCP](03-mcp.cs.md) (jak se Claude připojuje
k datům) a [Pluginy](04-plugins.cs.md) (jak se skills a příkazy balí
a instalují). Pokud jsou ti tyto pojmy nové, přečti si je nejdřív.

## Co placené řešení doopravdy je

Prodejní materiál popisuje balíček čtyř věcí:

| Část | Co to je | Platí se navíc? |
|------|----------|-----------------|
| Claude for Enterprise + Claude Code | Stejný Claude s vyššími limity, SSO a administrací | Ano — Enterprise smlouva |
| Předpřipravené MCP konektory | Napojení na FactSet, S&P Global, PitchBook, Morningstar, Moody's, LSEG, Daloopa… | Konektor je zdarma; **data vyžadují vlastní předplatné** u každého poskytovatele |
| Finanční skills a agenti | DCF, comps, LBO, earnings notes, IC memo, GL rekonciliace… | **Ne — open source** (Apache 2.0) |
| Onboarding služby | Pomoc s implementací od Anthropicu a partnerů | Ano |

Hlavní pointa: **workflow** (skills, příkazy, agenti) jsou otevřeně
zveřejněné na GitHubu. Platí se za škálu, enterprise administraci a prémiová
tržní data.

## Co můžeš používat bez balíčku

Potřebuješ běžné předplatné Claude, které zahrnuje Claude Code nebo Cowork
(aktuální plány viz [claude.com/pricing](https://claude.com/pricing)).
Všechno níže už je pak zdarma.

### 1. Samotný Claude

I bez pluginu Claude umí:

- číst PDF, Excel a CSV — výroční zprávy, bankovní exporty, obratové předvahy
- spouštět kód nad daty — ukazatele, růsty, prognózy, grafy
- vytvářet skutečné soubory `.xlsx`, `.pptx` a `.docx`
- dělat rešerše na webu s citací zdrojů

### 2. Open-source finanční pluginy od Anthropicu

Existují dva marketplaces pluginů. Každý je GitHub repo, které jednou
přidáš a pak z něj instaluješ jednotlivé pluginy.

**a) [`anthropics/financial-services`](https://github.com/anthropics/financial-services)**
— workflow pro kapitálové trhy (banking, research, PE, fund admin).

| Plugin | Přidává |
|--------|---------|
| `financial-analysis` *(instaluj první)* | `/comps`, `/dcf`, `/lbo`, `/3-statement-model`, `/competitive-analysis`, `/debug-model`, `/ppt-template`; skills na audit a čištění Excelu; všechny datové konektory |
| `equity-research` | `/earnings`, `/earnings-preview`, `/initiate`, `/model-update`, `/morning-note`, `/thesis`, `/catalysts`, `/sector`, `/screen` |
| `investment-banking` | `/cim`, `/teaser`, `/one-pager`, `/buyer-list`, `/merger-model`, `/process-letter`, `/deal-tracker` |
| `private-equity` | `/source`, `/screen-deal`, `/dd-checklist`, `/dd-prep`, `/ic-memo`, `/returns`, `/unit-economics`, `/value-creation`, `/portfolio` |
| `fund-admin` | Skills pro GL rekonciliaci, dohledání rozdílů, časové rozlišení, roll-forwardy, NAV tie-out, komentáře k odchylkám |
| Agenti (`pitch-agent`, `model-builder`, `earnings-reviewer`, `gl-reconciler`, `month-end-closer`, …) | Workflow od začátku do konce, které v sobě balí skills výše |

**b) [`anthropics/knowledge-work-plugins`](https://github.com/anthropics/knowledge-work-plugins)**
→ plugin `finance` — firemní účetnictví a FP&A.

| Příkaz | Dělá |
|--------|------|
| `/journal-entry` | Účetní zápisy — dohadné položky, náklady příštích období, odpisy, mzdy (MD/D) |
| `/reconciliation` | Hlavní kniha vs. pomocná evidence / banka, včetně rekonciliačních položek |
| `/income-statement` | Výkaz zisku a ztráty se srovnáním období |
| `/variance-analysis` | Plán vs. skutečnost, rozklad cena/objem, waterfall |
| `/sox-testing` | Pracovní papíry pro testování kontrol a výběr vzorků |

K tomu skills na pozadí pro měsíční uzávěrku, účetní výkazy a podporu auditu,
které si Claude načte sám, když jsou relevantní.

### 3. Bezplatné zdroje dat

Placené konektory jsou pohodlné, ne nutné. Claude umí pracovat s:

- **Vlastními soubory** — nahraj je nebo vlož do pracovní složky. Každý
  příkaz výše umí pracovat s vloženými nebo nahranými daty.
- **SEC EDGAR** (firmy kótované v USA) — zdarma, bez klíče. Claude Code ho
  umí volat přímo přes `curl`; MCP server není potřeba. SEC vyžaduje hlavičku
  `User-Agent` s kontaktním e-mailem a max. 10 požadavků za sekundu.
- **FRED** (makro řady USA i dalších zemí) — bezplatný API klíč na
  [fred.stlouisfed.org](https://fred.stlouisfed.org/docs/api/api_key.html).
- **API centrálních bank a statistických úřadů** — ECB, Eurostat i národní
  banky zveřejňují kurzy a makro data otevřeně.

Pokud zdroj používáš často, zabal ho jako MCP server a přidej do `.mcp.json`
— viz [Ukázka 3](03-mcp.cs.md), včetně toho, jak držet API klíč v proměnné
prostředí místo v souboru.

## Instalace pluginů

### Claude Code (CLI nebo záložka Code v desktopové aplikaci)

```bash
claude plugin marketplace add anthropics/financial-services
```

```bash
claude plugin install financial-analysis@claude-for-financial-services
```

Další vertikály přidáš stejně, například:

```bash
claude plugin install equity-research@claude-for-financial-services
```

Účetní plugin:

```bash
claude plugin marketplace add anthropics/knowledge-work-plugins
```

```bash
claude plugin install finance@knowledge-work-plugins
```

Spusť novou session a napiš `/` — nové příkazy se objeví v seznamu.

### Cowork

**Settings → Plugins → Add plugin**, vlož URL repa (např.
`https://github.com/anthropics/financial-services`) a vyber pluginy, které
chceš. Datové konektory se v Coworku spravují v
[claude.ai → Settings → Connectors](https://claude.ai/settings/connectors).

## Vyzkoušej: DCF bez placených dat (10 minut)

1. Nainstaluj `financial-analysis` (viz výše) a otevři Claude Code v prázdné
   složce.
2. Zeptej se:
   *„Download Apple's last 5 years of revenue, operating income, capex and
   free cash flow from SEC EDGAR companyfacts (CIK 0000320193). Use a
   User-Agent with my email. Save it as a CSV."*
3. Spusť `/dcf` a nasměruj ho na CSV. Claude postaví model v Excelu —
   zkontroluj, že buňky obsahují **vzorce**, ne vložená čísla.
4. Spusť `/debug-model` nad výsledkem, ať Claude zkontroluje vlastní sešit.
5. Požádej o jednostránkové shrnutí jako `.pptx` nebo `.docx`.

Co jsi právě použil: model (Claude), skill (`dcf-model`), příkaz (`/dcf`)
a bezplatný zdroj dat volaný přes shell — stejné vrstvy, jaké používá placené
řešení, jen bez prémiového datového feedu.

## Kde to funguje

| Prostředí | Pluginy | Konektory / data |
|-----------|---------|------------------|
| **Claude Code CLI** | ✅ `claude plugin install …` | ✅ `.mcp.json` a přímý `curl` na bezplatná API |
| **Desktopová aplikace — záložka Code** | ✅ stejně jako CLI | ✅ stejně jako CLI |
| **Cowork** | ✅ Settings → Plugins | ✅ Connectors v účtu; projektový `.mcp.json` ne |

## Limity a dobrá praxe

- **Není to poradenství.** Repozitáře Anthropicu samy uvádějí, že výstupy
  jsou koncepty ke kontrole kvalifikovaným člověkem. Claude připraví modely
  a memo; podepisuje je člověk.
- **Kontroluj čísla.** Ověř namátkou každý vstup modelu proti zdrojovému
  výkazu. Požádej Clauda, ať u každého vstupu uvede buňku nebo řádek výkazu.
- **Preferuj vzorce.** Chtěj živé excelové vzorce, aby šel model auditovat
  a měnit — ne list natvrdo zadaných hodnot.
- **Licence dat.** Data od placeného poskytovatele zůstávají pod jeho
  podmínkami i poté, co je Claude zpracuje.
- **Citlivá data.** Než nahraješ klientská nebo neveřejná data, zkontroluj
  nastavení využití dat ve svém plánu a firemní pravidla pro schválené AI
  nástroje.

## Řešení problémů

- **Příkaz po instalaci neexistuje** → spusť novou session; `claude plugin
  list` ověří, že je plugin zapnutý.
- **Konektor hlásí chybu přihlášení** → poskytovatel vyžaduje vlastní
  předplatné nebo API klíč; použij nahrané soubory nebo bezplatný zdroj.
- **EDGAR vrací 403** → požadavku chybí hlavička `User-Agent` s kontaktním
  e-mailem.
- **Čísla v modelu nesedí** → požádej Clauda o seznam všech vstupů se
  zdrojem a porovnej je s výkazem.
