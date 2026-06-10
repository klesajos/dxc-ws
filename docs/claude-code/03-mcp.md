# Example 3: Project-scoped MCP connections

**What it is:** MCP (Model Context Protocol) servers give Claude extra tools —
documentation lookups, databases, browsers, ticketing systems. A `.mcp.json`
at the repo root makes the connections **project-scoped**: checked in, shared
by everyone who clones the repo.

**Where it lives:**

```
.mcp.json
```

## The two servers (one per transport type)

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

- **`deepwiki` (HTTP transport)** — a remote server, reached over HTTPS. It
  answers questions about any public GitHub repo's documentation. No auth
  needed, which makes it ideal for demos.
- **`memory` (stdio transport)** — a local process Claude Code spawns via
  `npx`. It gives Claude a persistent knowledge graph (entities/relations)
  for the session. Shows the command/args pattern used by most local servers.

Neither needs credentials. For servers that do, reference environment
variables instead of hardcoding secrets: `"headers": { "Authorization":
"Bearer ${API_TOKEN}" }` — never commit tokens to `.mcp.json`.

## Security note

Project-scoped servers run code/connect on every collaborator's machine, so
Claude Code asks each user to **approve** `.mcp.json` servers the first time
they open the project. Run `/mcp` to see status, tools, and re-authenticate.

## Demo

1. Open Claude Code in the repo → approve the two project servers.
2. Run `/mcp` — both servers and their tools are listed.
3. Ask: *"Use deepwiki to look up how generators work in catchorg/Catch2,
   then add a generator-based test for slideLineLeft."* — an external doc
   lookup feeding directly into project work.
