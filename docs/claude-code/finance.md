> 🌍 Read this in: **English** | [Česky](finance.cs.md)

# Using Claude for finance work

Anthropic sells a **"Claude for Financial Services"** solution aimed at
banks, asset managers and PE funds. This guide explains what is inside it,
which parts cost extra, and how to use the parts that don't — plugins,
skills and free data sources — from Claude Code or Cowork.

It builds on two earlier examples: [MCP](03-mcp.md) (how Claude connects to
data) and [Plugins](04-plugins.md) (how skills and commands are packaged and
installed). Read those first if the terms are new.

## What the paid solution actually is

The sales material describes a bundle of four things:

| Part | What it is | Costs extra? |
|------|------------|--------------|
| Claude for Enterprise + Claude Code | The same Claude, with higher usage limits, SSO, admin controls | Yes — Enterprise contract |
| Pre-built MCP connectors | Links to FactSet, S&P Global, PitchBook, Morningstar, Moody's, LSEG, Daloopa… | The connector is free; **the data needs your own subscription** with each provider |
| Finance skills and agents | DCF, comps, LBO, earnings notes, IC memos, GL reconciliation… | **No — open source** (Apache 2.0) |
| Onboarding services | Implementation help from Anthropic and partners | Yes |

The key point: the **workflows** (skills, commands, agents) are published
openly on GitHub. What you pay for is scale, enterprise administration and
premium market data.

## What you can use without the bundle

You need a normal Claude subscription that includes Claude Code or Cowork
(check the current plans at [claude.com/pricing](https://claude.com/pricing)).
On top of that, everything below costs nothing.

### 1. Claude itself

Without any plugin, Claude can already:

- read PDFs, Excel and CSV files — annual reports, bank exports, trial balances
- run code on the data — ratios, growth rates, forecasts, charts
- produce real `.xlsx`, `.pptx` and `.docx` files
- research on the web and cite sources

### 2. Anthropic's open-source finance plugins

There are two plugin marketplaces. Each is a GitHub repo you add once, then
install individual plugins from.

**a) [`anthropics/financial-services`](https://github.com/anthropics/financial-services)**
— capital-markets workflows (banking, research, PE, fund admin).

| Plugin | Adds |
|--------|------|
| `financial-analysis` *(install first)* | `/comps`, `/dcf`, `/lbo`, `/3-statement-model`, `/competitive-analysis`, `/debug-model`, `/ppt-template`; Excel audit and data-cleaning skills; all the data connectors |
| `equity-research` | `/earnings`, `/earnings-preview`, `/initiate`, `/model-update`, `/morning-note`, `/thesis`, `/catalysts`, `/sector`, `/screen` |
| `investment-banking` | `/cim`, `/teaser`, `/one-pager`, `/buyer-list`, `/merger-model`, `/process-letter`, `/deal-tracker` |
| `private-equity` | `/source`, `/screen-deal`, `/dd-checklist`, `/dd-prep`, `/ic-memo`, `/returns`, `/unit-economics`, `/value-creation`, `/portfolio` |
| `fund-admin` | Skills for GL reconciliation, break tracing, accruals, roll-forwards, NAV tie-out, variance commentary |
| Agents (`pitch-agent`, `model-builder`, `earnings-reviewer`, `gl-reconciler`, `month-end-closer`, …) | End-to-end workflows that bundle the skills above |

**b) [`anthropics/knowledge-work-plugins`](https://github.com/anthropics/knowledge-work-plugins)**
→ `finance` plugin — corporate accounting and FP&A.

| Command | Does |
|---------|------|
| `/journal-entry` | Accruals, prepaids, depreciation, payroll entries with debits/credits |
| `/reconciliation` | GL vs. subledger / bank, with reconciling items |
| `/income-statement` | P&L with period-over-period comparison |
| `/variance-analysis` | Budget vs. actual, price/volume decomposition, waterfall |
| `/sox-testing` | Control-testing workpapers and sample selection |

Plus background skills for month-end close, financial statements and audit
support that Claude loads automatically when relevant.

### 3. Free data sources

The paid connectors are convenient, not required. Claude can work with:

- **Your own files** — upload or drop them in the working folder. Every
  command above falls back to pasted or uploaded data.
- **SEC EDGAR** (US listed companies) — free, no key. Claude Code can call it
  directly with `curl`; no MCP server needed. SEC asks for a `User-Agent`
  header with your contact email and max 10 requests per second.
- **FRED** (US and international macro series) — free API key from
  [fred.stlouisfed.org](https://fred.stlouisfed.org/docs/api/api_key.html).
- **Central bank / statistics office APIs** — ECB, Eurostat, national banks
  publish exchange rates and macro data openly.

If you use a source often, wrap it as an MCP server and add it to
`.mcp.json` — see [Example 3](03-mcp.md), including how to keep the API key
in an environment variable instead of the file.

## Install the plugins

### Claude Code (CLI or Desktop Code tab)

```bash
claude plugin marketplace add anthropics/financial-services
```

```bash
claude plugin install financial-analysis@claude-for-financial-services
```

Add the verticals you need the same way, for example:

```bash
claude plugin install equity-research@claude-for-financial-services
```

For the accounting plugin:

```bash
claude plugin marketplace add anthropics/knowledge-work-plugins
```

```bash
claude plugin install finance@knowledge-work-plugins
```

Start a new session and type `/` — the new commands appear in the list.

### Cowork

**Settings → Plugins → Add plugin**, paste the repo URL (for example
`https://github.com/anthropics/financial-services`) and pick the plugins you
want. Data connectors in Cowork are managed under
[claude.ai → Settings → Connectors](https://claude.ai/settings/connectors).

## Try it: a DCF with no paid data (10 minutes)

1. Install `financial-analysis` (above) and open Claude Code in an empty
   folder.
2. Ask:
   *"Download Apple's last 5 years of revenue, operating income, capex and
   free cash flow from SEC EDGAR companyfacts (CIK 0000320193). Use a
   User-Agent with my email. Save it as a CSV."*
3. Run `/dcf` and point it at the CSV. Claude builds the model in an Excel
   file — check that the cells contain **formulas**, not pasted numbers.
4. Run `/debug-model` on the result to have Claude audit its own workbook.
5. Ask for a one-page summary as `.pptx` or `.docx`.

What you just used: the model (Claude), a skill (`dcf-model`), a command
(`/dcf`), and a free data source called through the shell — the same layers
the paid solution uses, minus the premium data feed.

## Where it works

| Surface | Plugins | Connectors / data |
|---------|---------|-------------------|
| **Claude Code CLI** | ✅ `claude plugin install …` | ✅ `.mcp.json`, plus direct `curl` to free APIs |
| **Desktop app — Code tab** | ✅ same as CLI | ✅ same as CLI |
| **Cowork** | ✅ Settings → Plugins | ✅ account Connectors; no project `.mcp.json` |

## Limits and good practice

- **Not advice.** Anthropic's own repos state the outputs are drafts for a
  qualified person to review. Claude prepares models and memos; a human
  signs them off.
- **Check the numbers.** Spot-check every model input against the source
  filing. Ask Claude to cite the source cell or filing line for each input.
- **Prefer formulas.** Ask for live Excel formulas so the model can be
  audited and changed, not a sheet of hard-coded values.
- **Data licensing.** Data from a paid provider stays under that provider's
  terms, even after Claude has processed it.
- **Sensitive data.** Before uploading client or non-public data, check
  your plan's data-use settings and your firm's policy on approved AI tools.

## Troubleshooting

- **Command not found after install** → start a new session; run
  `claude plugin list` to confirm the plugin is enabled.
- **Connector shows an auth error** → that provider needs your own
  subscription or API key; use uploaded files or a free source instead.
- **EDGAR returns 403** → the request is missing a `User-Agent` header with a
  contact email.
- **Model numbers look wrong** → ask Claude to list every input with its
  source, then compare against the filing.
