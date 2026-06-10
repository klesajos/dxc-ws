# Example 4: Project-scoped plugin

**What it is:** a plugin bundles Claude Code extensions (commands, skills,
agents, hooks, MCP servers) into one installable, versioned package,
distributed through a **marketplace**. This repo is its own marketplace, so
the plugin ships with the project.

**Where it lives:**

```
.claude-plugin/marketplace.json      # marketplace catalog (repo root)
plugins/2048-dev/
  .claude-plugin/plugin.json         # plugin manifest
  commands/build-test.md             # the /2048-dev:build-test command
.claude/settings.json                # auto-registers + enables the plugin
```

## The plugin

`2048-dev` ships a single slash command:

- **`/2048-dev:build-test`** — configures, builds, runs `ctest`, and reports
  a pass/fail summary with failing assertions. Status only, no fixes.

Commands are Markdown files: frontmatter (`description`, `allowed-tools`)
plus the prompt Claude executes. Plugins can also carry `skills/`, `agents/`,
and `hooks/` — same formats as the standalone examples 1 and 2.

## The marketplace

`.claude-plugin/marketplace.json` at the repo root is a catalog: it names the
marketplace (`dxc-ws`) and lists plugins with their sources — here a relative
path into the same repo.

## Auto-enable for collaborators

These two keys in `.claude/settings.json` (checked in) make the plugin
appear for everyone who clones the repo, after a one-time trust prompt:

```json
{
  "extraKnownMarketplaces": {
    "dxc-ws": { "source": { "source": "github", "repo": "klesajos/dxc-ws" } }
  },
  "enabledPlugins": { "2048-dev@dxc-ws": true }
}
```

## Plugin vs. standalone skill/hook — when to use which

- **Standalone** (`.claude/skills/`, settings hooks): one project, zero
  ceremony. Examples 1–2.
- **Plugin**: versioned, reusable across repos and teams, installable with
  one command, can bundle many pieces. The natural next step once a skill or
  hook proves useful beyond a single project.

## Demo

1. Open Claude Code in a fresh clone → accept the marketplace trust prompt.
2. Type `/2048-dev:` — the `build-test` command autocompletes.
3. Run `/2048-dev:build-test` and watch it build + test + summarize.
4. For local iteration on the plugin itself: `/plugin marketplace add ./`
   then `/plugin install 2048-dev@dxc-ws`.
