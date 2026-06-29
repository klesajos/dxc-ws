> 🌍 Read this in: **English** | [Česky](03-mcp.cs.md)

# Example 3: Project-scoped MCP connections

## What is MCP?

**MCP (Model Context Protocol)** is a standard for connecting AI assistants
to external tools. Think of it as a universal adapter — like USB-C: one
standard socket that lets Claude plug into any external tool or data source.
An **MCP server** is a small program that offers Claude
extra abilities — querying a database, searching documentation, controlling
a browser, reading your ticketing system.

Out of the box, Claude Code can read/edit files and run terminal commands.
MCP is how you give it *everything else*.

## What this example does

The file `.mcp.json` in the repo root connects two servers. Because the
file is committed to git, the connections are **project-scoped**: every
person who clones the repo gets them (after a one-time approval — see
the security note below).

## The file, line by line

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

- `"mcpServers"` — the top-level key; each entry inside is one server.
- `"deepwiki"` / `"memory"` — names you choose. They show up in `/mcp` and
  in tool names.

**The `deepwiki` server — `http` transport (a remote service):**

- `"type": "http"` — the server runs somewhere on the internet; Claude
  talks to it over HTTPS. Nothing runs on your machine.
- `"url"` — the server's address. DeepWiki answers questions about the
  documentation of any public GitHub repository, no account needed.

**The `memory` server — `stdio` transport (a local program):**

- `"type": "stdio"` — Claude Code **starts the program on your machine**
  and talks to it through standard input/output. This is the most common
  type for local tools.
- `"command": "npx"` — the program to launch. `npx` is part of Node.js and
  runs an npm package without a permanent install.
- `"args": ["-y", "@modelcontextprotocol/server-memory"]` — arguments
  passed to the command, exactly like typing
  `npx -y @modelcontextprotocol/server-memory` in a terminal. `-y` skips
  npx's "install this package?" confirmation. The memory server gives
  Claude a small knowledge graph to store facts in during a session.

## What about servers that need a password or API key?

**Never write secrets into `.mcp.json`** — the file is committed and would
leak the key to everyone. Reference an environment variable instead:

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

`${MY_API_TOKEN}` is replaced at runtime with the value of that environment
variable from your shell — each developer keeps their own key locally
(e.g. in `~/.zshrc` or an `.env` file that is in `.gitignore`).

## Security note: the approval prompt

A project `.mcp.json` means *someone else's config can run programs on your
machine* (the `stdio` type literally launches a process). That's why Claude
Code asks you to **approve the project's servers the first time** you open
the repo. Review what's in the file before approving — exactly what you're
doing now by reading this guide.

## Add your own server, step by step

1. **Find a server.** Browse the registry at
   [github.com/modelcontextprotocol/servers](https://github.com/modelcontextprotocol/servers)
   or your vendor's docs (GitHub, Sentry, Postgres... most have one).
2. **Add an entry to `.mcp.json`** following one of the two patterns above
   (`http` for hosted servers, `stdio` for local ones).
3. **Check the JSON is valid** — a missing comma is the #1 mistake:
   ```bash
   jq . .mcp.json
   ```
   If `jq` prints the file back, it's valid; if it prints an error, fix the
   syntax.
4. **Start a new Claude Code session** in the repo and approve the server.
5. **Verify with `/mcp`** — the command lists every server, its status,
   and the tools it provides.
6. **Commit the file.**

## Try the demo

1. Type `/mcp` — you should see `deepwiki` and `memory`, both connected.
2. Ask: *"Use deepwiki to look up how generators work in catchorg/Catch2,
   then add a generator-based test for slideLineLeft."*
   Claude calls the external documentation tool and feeds the answer
   straight into project work.

## Optional parameters

Our two entries are minimal. Useful optional fields per transport type:

**For `stdio` servers (local programs):**

| Field | What it does |
|-------|--------------|
| `env` | Environment variables passed to the server process, e.g. `{"DEBUG": "true"}` |
| `cwd` | Working directory the server starts in (defaults to where Claude runs) |

**For `http` servers (remote services):**

| Field | What it does |
|-------|--------------|
| `headers` | Custom HTTP headers — typically `Authorization` for API keys |
| `timeout` | Per-server timeout for tool calls, in milliseconds |
| `oauth` | OAuth configuration for servers that use a browser login flow |

**Environment variable expansion** works in `command`, `args`, `env`,
`url`, and `headers`:

- `${VAR}` — replaced with the value of `VAR` from your shell
- `${VAR:-default}` — uses `default` when `VAR` isn't set

Full reference: [official MCP documentation](https://code.claude.com/docs/en/mcp).

## Where it works: CLI, Desktop app, Cowork

| Platform | Works? | Setup |
|----------|--------|-------|
| **Claude Code CLI** (terminal) | ✅ Yes | Nothing extra — `.mcp.json` is read from the repo root; approve the servers on first session |
| **Claude Desktop app — Code tab** | ✅ Yes | Same engine and config as the CLI. Approve the project trust dialog, then both servers appear in `/mcp` |
| **Cowork** (in the Desktop app) | ❌ No (different mechanism) | Cowork does not load project `.mcp.json`. Instead it uses **Connectors** configured in your Claude account ([claude.ai → Settings → Connectors](https://claude.ai/settings/connectors)). Add the server there and it's available to Cowork tasks — but it's account-scoped, not project-scoped, and applies to all your Cowork sessions |

Practical rule: `.mcp.json` = *this repo, everyone who clones it*;
Connectors = *your account, every Cowork/claude.ai conversation*. Same
protocol underneath (MCP), different distribution.

## Troubleshooting

- **Server missing in `/mcp`** → invalid JSON (run `jq . .mcp.json`) or
  you declined the approval prompt — run `claude mcp reset-project-choices`
  and reopen.
- **`memory` fails to start** → you need Node.js installed (`node --version`);
  the first `npx` run also needs internet to download the package.
- **`deepwiki` times out** → check network/proxy; the server is remote.
