> 🌍 Read this in: **English** | [Česky](04-plugins.cs.md)

# Example 4: Project-scoped plugin

## What is a plugin?

A **plugin** bundles Claude Code extensions — slash commands, skills,
agents, hooks, MCP servers — into one installable, versioned package.
Plugins are distributed through a **marketplace**: a catalog file that says
"these plugins exist and here is where to get them".

In examples 1–3 you put a skill, a hook, and MCP servers directly into the
repo. That works great for *one* project. A plugin is the next step: the
same content packaged so it can be shared across many repos and teams.

This example shows the smallest possible setup: the repo is **its own
marketplace** offering **one plugin** with **one command**.

## The moving parts

```
.claude-plugin/marketplace.json      ← the marketplace catalog (repo root)
plugins/2048-dev/                    ← the plugin itself
  .claude-plugin/plugin.json         ← plugin manifest (name, version, ...)
  commands/build-test.md             ← one slash command
.claude/settings.json                ← auto-registers + enables the plugin
```

Three layers, from inside out: a **command** lives in a **plugin**, which is
listed in a **marketplace**, which the project's **settings** point to.

## Layer 1: the command (`plugins/2048-dev/commands/build-test.md`)

A slash command is a Markdown file. The file name = the command name
(`build-test.md` → `/2048-dev:build-test`; the prefix is the plugin name).

```markdown
---
description: Configure, build, and run the full test suite, then summarize results
allowed-tools: Bash(cmake:*), Bash(ctest:*)
---

Build the project and run the tests:

1. Configure if needed: `cmake -S . -B build`
...
```

- `description` — shown in the command autocomplete list.
- `allowed-tools` — pre-approves the tools the command needs:
  `Bash(cmake:*)` means "any `cmake` command" may run without asking for
  permission. Scope this narrowly — *which* tools, not "all tools".
- The body is the **prompt** Claude executes when you type the command.
  Write it like a precise work order: numbered steps and what to report.

## Layer 2: the plugin manifest (`plugins/2048-dev/.claude-plugin/plugin.json`)

```json
{
  "name": "2048-dev",
  "description": "Dev workflow commands for the 2048 workshop project",
  "version": "1.0.0",
  "author": { "name": "Josef Klesa", "url": "https://github.com/klesajos" }
}
```

- `name` — the plugin's identifier; becomes the command prefix
  (`/2048-dev:...`). Only required field.
- `version` — lets users see when the plugin changed and update.
- The manifest must sit in a `.claude-plugin/` folder **inside the plugin
  directory**. Content folders (`commands/`, `skills/`, `agents/`,
  `hooks/`) sit **next to** `.claude-plugin/`, not inside it — that's the
  most common beginner mistake.

## Layer 3: the marketplace (`.claude-plugin/marketplace.json` at repo root)

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

- `name` — the marketplace's identifier; appears after the `@` in
  `2048-dev@dxc-ws`.
- `owner` — who maintains the catalog.
- `plugins` — the list of offered plugins. `source: "./plugins/2048-dev"`
  is a **relative path within this repo** — the plugin ships with the
  project. A source can also point elsewhere (another GitHub repo, npm),
  which is how companies run one central marketplace for many plugins.

## Layer 4: auto-enable (`.claude/settings.json`)

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

- `extraKnownMarketplaces` — "this project uses the marketplace called
  `dxc-ws`, fetch it from the GitHub repo `klesajos/dxc-ws`". Anyone
  opening the project gets the marketplace registered automatically
  (after a trust prompt).
- `enabledPlugins` — "from that marketplace, turn on `2048-dev`". The key
  format is `<plugin-name>@<marketplace-name>`.

Together: clone the repo → open Claude Code → accept one prompt →
`/2048-dev:build-test` just works.

## Build your own plugin, step by step

1. **Create the plugin skeleton:**
   ```bash
   mkdir -p plugins/my-plugin/.claude-plugin plugins/my-plugin/commands
   ```
2. **Write the manifest** `plugins/my-plugin/.claude-plugin/plugin.json` —
   copy the Layer 2 example, change `name` and `description`.
3. **Add a command** `plugins/my-plugin/commands/hello.md` — frontmatter
   with a `description`, body with the instructions (see Layer 1).
4. **List it in the marketplace** — add an entry to the `plugins` array in
   `.claude-plugin/marketplace.json` with `source: "./plugins/my-plugin"`.
5. **Validate** — Claude Code ships a checker:
   ```bash
   claude plugin validate .
   ```
   Fix anything it reports before continuing.
6. **Test locally** inside Claude Code:
   ```
   /plugin marketplace add ./
   /plugin install my-plugin@dxc-ws
   ```
   Then type `/my-plugin:hello` to run your command.
7. **Enable it for everyone** — add `"my-plugin@dxc-ws": true` to
   `enabledPlugins` in `.claude/settings.json`.
8. **Commit everything and push.**

## Plugin vs. standalone skill/hook — which to choose?

- **Standalone** (`.claude/skills/`, hooks in settings): one project, zero
  ceremony. Start here — it's examples 1 and 2.
- **Plugin**: versioned, shareable across repos, bundles many pieces,
  installable with one command. Move to a plugin when a skill or hook
  proves useful beyond a single project.

## Try the demo

1. Open Claude Code in a fresh clone → accept the marketplace trust prompt.
2. Type `/2048-dev:` — `build-test` autocompletes.
3. Run it. Claude configures, builds, runs `ctest`, and reports a pass/fail
   summary without fixing anything — exactly what the command file asked.

## Troubleshooting

- **Command doesn't autocomplete** → run `claude plugin validate .`;
  check the plugin is enabled in `/plugin` → "Manage plugins".
- **"Marketplace not trusted"** → you declined the prompt; re-add manually
  with `/plugin marketplace add ./`.
- **Changed the command file but nothing happens** → start a new session;
  plugin content is loaded at session start.
