> 🌍 Read this in: **English** | [Česky](08-output-styles.cs.md)

# Example 8: Project output style

## What is an output style?

An **output style** changes **how Claude talks to you**, not what it can do.
It's a Markdown file whose body is added to Claude's system prompt. Compare it
to the other mechanisms:

- A **skill** adds knowledge for one kind of task, loaded on demand.
- A **rule** or `CLAUDE.md` adds project facts and conventions.
- An **output style** shapes *every* answer: its length, structure and tone.

Claude Code ships five built-in styles:

| Style | What it does |
|-------|--------------|
| **Default** | Claude Code's standard software-engineering prompt, with no style added |
| **Proactive** | Starts work right away and makes reasonable assumptions instead of asking about routine decisions |
| **Concise** | Leads with the result and leaves out preamble, narration and recaps |
| **Explanatory** | Adds short `★ Insight` blocks that explain the choices behind the code |
| **Learning** | Explains choices and leaves small pieces of code for you to write |

`/output-style` was briefly deprecated and came back in 2.1.269. Concise was
added in 2.1.237.

## What this example does

The style `workshop-tutor` makes every answer follow the same three-part
shape: **What I did**, **Why it works** and **Try it**. A whole workshop room
gets answers it can compare side by side. Each answer ends with a command the
participant can run, so nobody just reads along.

## The file, line by line

The style lives at `.claude/output-styles/workshop-tutor.md`:

```
.claude/
└── output-styles/          ← project output styles (committed to git)
    └── workshop-tutor.md   ← file name = style name, unless `name:` overrides it
```

```markdown
---
name: workshop-tutor
description: Explains each change for workshop participants and ends with a hands-on "Try it" step
keep-coding-instructions: true
---

# Workshop tutor

You are pairing with a participant of a Claude Code workshop on the 2048 C++
project. Keep doing the engineering work as usual, but shape every answer like
this:

1. **What I did** — one or two sentences, naming the files and functions you
   touched as `file:line`.
2. **Why it works** — the C++ or Claude Code concept behind the change, in at
   most three bullets. Skip anything a working developer already knows.
3. **Try it** — one concrete command or prompt the participant can run next
   to see the result for themselves (for example `./build/2048` or
   `ctest --test-dir build --output-on-failure`).

Keep the tone plain and direct. No praise, no filler.
```

What each line does:

- `name:` — the name shown in `/output-style` and `/config`. Optional; the
  file name is used if you leave it out.
- `description:` — shown next to the name in the picker.
- `keep-coding-instructions: true` — **the line that matters most.** A
  custom style *replaces* Claude Code's built-in software-engineering
  instructions (how to scope changes, write comments, verify work) unless
  you set this. We still want Claude to code the same way, just to talk
  differently, so it's on.
- The body is the instruction Claude follows for every answer.

## Create your own output style, step by step

1. **Create the folder:**
   ```bash
   mkdir -p .claude/output-styles
   ```

2. **Create the file** `.claude/output-styles/my-style.md`:
   ```markdown
   ---
   description: <one line shown in the picker>
   keep-coding-instructions: true
   ---

   <How every answer should look: structure, length, tone.>
   ```

3. **Restart Claude Code.** Style files are read at startup, so a style you
   create or edit mid-session appears only after a restart.

4. **Switch to it:**
   ```text
   /output-style my-style
   ```
   Run `/output-style` with no argument to list all styles. The current one is
   marked. The new style applies from your **next** message.

5. **Commit it** so the whole team can pick it:
   ```bash
   git add .claude/output-styles/my-style.md
   git commit -m "Add my-style output style"
   ```
   Committing makes the style *available*, not *active*. Each person selects
   it themselves. The `/config` menu saves that choice to
   `.claude/settings.local.json`, your personal file that stays out of git.

## Try the demo

```text
/output-style workshop-tutor
```

Then ask: *"What does Board::hasWon() return? Don't edit anything."*

The answer comes back in three labelled parts and ends with a **Try it**
command such as `grep -n "hasWon" tests/test_board.cpp`. Switch back with
`/output-style default`.

To run the same check without an interactive session:

```bash
claude -p --settings '{"outputStyle":"workshop-tutor"}' \
  "What does Board::hasWon() return? Don't edit anything."
```

## Optional parameters

| Field / setting | Where | What it does |
|-----------------|-------|--------------|
| `name` | style frontmatter | Display name. Default: file name |
| `description` | style frontmatter | Text shown in the picker |
| `keep-coding-instructions` | style frontmatter | `true` keeps Claude Code's built-in engineering instructions. Default `false` |
| `force-for-plugin` | style frontmatter, **plugin styles only** | `true` turns the style on whenever the plugin is enabled, overriding the user's choice |
| `outputStyle` | any `settings.json` | Selects a style without the menu. **Case-sensitive**: `"Explanatory"`, not `"explanatory"`. A value that doesn't match falls back to Default |

Styles load from `~/.claude/output-styles/` (personal), from every
`.claude/output-styles/` between your working directory and the repo root
(project), and from enabled plugins. Full reference:
[official output styles documentation](https://code.claude.com/docs/en/output-styles).

## Where it works: CLI, Desktop app, Cowork

| Platform | Works? | Setup |
|----------|--------|-------|
| **Claude Code CLI** (terminal) | ✅ Yes | `/output-style <name>`, or `/config` → **Output style** |
| **Claude Desktop app — Code tab** | ✅ Yes | Set `outputStyle` in `.claude/settings.local.json`. In Desktop, `/config` opens **Settings > Claude Code** instead of a menu |
| **VS Code extension** | ✅ Yes | `/` → **Output styles**, custom styles included (2.1.257+) |
| **Cowork** (in the Desktop app) | ❌ No | Project `.claude/` config doesn't load. Ship the style in a plugin instead |

## Troubleshooting

- **Style not in the `/output-style` list** → the file must be in
  `.claude/output-styles/` with a `.md` extension. Restart Claude Code after
  creating it.
- **Claude stopped running tests or started making sloppy edits after
  switching** → `keep-coding-instructions: true` is missing, so the style
  replaced the engineering instructions.
- **`outputStyle` in settings has no effect** → check the case. The value
  must match the style name exactly.
