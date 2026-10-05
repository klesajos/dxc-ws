> 🌍 Read this in: **English** | [Česky](10-validate-eval.cs.md)

# Example 10: Validating and evaluating a plugin

## What are validation and evals?

Examples 1–9 *build* extensions. This one *checks* them. Claude Code has four
built-in quality tools, from cheapest to most thorough:

| Tool | Question it answers | Costs tokens? |
|------|---------------------|---------------|
| `claude plugin validate <path>` | Is the plugin structurally valid: manifest, frontmatter, file references? | No |
| `claude plugin details <name>` | What's inside the plugin, and what does it cost in context? | No |
| `/skill-doctor` | Which loaded skills never get used, and what do they cost? | No |
| `claude plugin eval <path>` | Does the plugin actually make Claude **better** at its job? | Yes, it runs real sessions |

An **eval** is the plugin equivalent of a unit test. You write a prompt plus
**graders** that score Claude's answer. `claude plugin eval` runs the prompt
in a hermetic sandbox **with** your plugin and **without** it, then reports
the difference (Δ). A positive Δ is evidence that the plugin earns its
context cost.

## What this example does

The eval suite `plugins/2048-dev/evals/explorer-trace/` checks the plugin's
`game-explorer` agent ([Example 5](05-agents.md)). It asks Claude to trace
how the Left arrow key reaches `Board::move`, then grades the answer three
ways. A verified run on Claude Code 2.1.283 scored **1.00 with the plugin
vs 0.33 without** (Δ +0.67, one run per arm, $0.24).

## The files, line by line

```
plugins/2048-dev/
└── evals/                      ← default eval directory of a plugin
    └── explorer-trace/         ← one directory per case
        ├── case.yaml           ← case metadata + setup
        ├── prompt.md           ← run settings (frontmatter) + the prompt (body)
        ├── setup.sh            ← scaffold script: prepares the workspace
        ├── fixture/            ← trimmed copy of the 2048 call chain
        └── graders/            ← one file per grader
            ├── criteria.md
            ├── explorer-used.md
            └── reads-input.md
```

**`case.yaml`** — required whenever you need `context.*` fields:

```yaml
schema_version: "1.1"
name: explorer-trace
context:
  scaffold_script: setup.sh
```

`schema_version` and `name` are mandatory in `case.yaml`. The run fails to
load without them.

**`setup.sh`** — every run starts in an **empty** temporary workspace with
no access to your repo. The scaffold script copies the fixture in:

```bash
#!/usr/bin/env bash
# Runs in the empty eval workspace before Claude starts (only with --scaffold).
# Copies the trimmed 2048 call chain into the workspace as src/.
set -euo pipefail
mkdir -p src
cp "$(dirname "$0")"/fixture/*.cpp src/
```

**`prompt.md`** — frontmatter sets the run, the body is sent verbatim:

```markdown
---
description: The game-explorer agent maps a keypress down to Board::move
max_turns: 15
allowed_tools: [Read, Glob, Grep, Agent]
tags: [smoke, agent]
---

The source code of a terminal 2048 game is in `src/`.
Trace how pressing the Left arrow key ends up calling `Board::move`.
Answer with an ordered list of `file:function` steps, from `main()` to
`Board::move`.
```

`allowed_tools` lists only read-only tools, so no `--allow-tools` grant is
needed. `Bash`, `Edit`, `Write` and `WebFetch` are gated and must be granted
on the command line.

**The three graders** use three different grader types:

```markdown
---
type: llm
weight: 2
---

PASS if the answer is an ordered list that starts at `main` (main.cpp), goes
through the game loop in game.cpp and the key reading in input.cpp, and ends at
`Board::move` in board.cpp, naming a file and a function at each step.
FAIL if any of those four files is missing, the order is wrong, or the answer
invents functions that are not in the source.
```

```markdown
---
type: tool_used
tool: Agent
input_match: "game-explorer"
arm: with-only
---

The plugin's game-explorer agent was delegated to.
```

```markdown
---
type: regex
pattern: "input\\.cpp"
weight: 1
---

The trace must pass through input.cpp, where the arrow key is decoded. A
reply that only says "I couldn't find Board::move" never names this file,
so it fails here.
```

- `llm` — a judge model (Haiku by default) votes three times. PASS needs
  two of three. Write the rubric as concrete PASS and FAIL conditions.
- `tool_used` with `arm: with-only` — "did the plugin's agent fire?" The
  no-plugin arm can't fire it, so this grader is reported but **not
  scored**.
- `regex` — free and deterministic. **Lesson from building this suite:** the
  first draft matched `Board::move`, and it passed on an answer that said
  "I can't find `Board::move`". Pick a pattern that only a *correct* answer
  contains.

## Create your own eval, step by step

1. **Scaffold a blank case** from the plugin root:
   ```bash
   cd plugins/my-plugin
   claude plugin eval init --bare my-case
   ```
   This writes `evals/my-case/prompt.md` and `evals/my-case/graders/criteria.md`.
   Without `--bare` you get an interactive interview that designs the graders
   with you.

2. **Write the prompt** in `prompt.md`. Phrase it the way a real user would.
   Don't name your skill or agent, because the eval should prove Claude picks
   it on its own.

3. **Write at least one grader** in `graders/`. Prefer cheap, deterministic
   graders (`regex`, `tool_used`, `file_exists`) and add one `llm` grader
   for quality.

4. **Need files in the workspace?** Add `case.yaml` with
   `context.scaffold_script` and run with `--scaffold`.

5. **Run it cheaply while iterating:**
   ```bash
   claude plugin eval . --scaffold --runs 1 --no-publish
   ```

6. **Ignore the results directory** (this repo's `.gitignore` already does):
   ```gitignore
   plugins/*/evals/results/
   ```

## Try the demo

Validate first. This is free and fast:

```bash
claude plugin validate plugins/2048-dev --json
claude plugin details 2048-dev
```

`details` shows the plugin's context bill: about 460 tokens always-on, almost
all of it the `game-explorer` description.

Then run the eval. It makes real API calls, about $0.25 per run pair:

```bash
cd plugins/2048-dev
claude plugin eval . --scaffold --runs 1 --no-publish
```

Expected output (scores vary between runs):

```text
✓ explorer-trace  with 1.00  without 0.33  Δ +0.67  (2 runs)  $0.24
```

The first run asks you to confirm that you trust the plugin. Pass
`--trust-plugin` in CI. Open the `report.html` path printed at the end for
per-grader verdicts and the judge's evidence.

## Optional parameters

| Flag | What it does |
|------|--------------|
| `--runs <n>` | Runs per arm (default: the case's `runs`, else 3). More runs mean a less noisy score |
| `--ablation none` | Skip the no-plugin arm. Half the cost, but no Δ |
| `--scaffold` | Run `scaffold_script`. Off by default because it runs your bash as you |
| `--allow-tools <tools...>` | Grant gated tools, e.g. `Bash(cmake:*)` |
| `--threshold <0..1>` | Exit 1 if any case scores below it (default 1.0). Use in CI |
| `--max-cost-usd <usd>` | Hard budget. Partial results are reported if it's hit |
| `--judge-model <model>` | Model for `llm` graders (default Haiku) |
| `--no-publish` | Keep the HTML report local instead of publishing it privately to claude.ai |
| `--json [path]` | Full machine-readable result |

Other grader types: `file_exists`, `tool_order` and `baseline`. MCP-backed
skills can be evaluated against **mock** servers in `evals/mocks/`. Full
reference: [official plugin evals documentation](https://code.claude.com/docs/en/plugin-evals).

## Where it works: CLI, Desktop app, Cowork

| Platform | Works? | Setup |
|----------|--------|-------|
| **Claude Code CLI** (terminal) | ✅ Yes | `claude plugin validate`, `details` and `eval` are CLI subcommands. `/skill-doctor` runs inside a session |
| **Claude Desktop app — Code tab** | ⚠️ Partly | The `claude plugin …` subcommands are CLI-only. Run them from a terminal in the same repo |
| **Cowork** (in the Desktop app) | ❌ No | Evaluate the plugin in the CLI before you install it in Cowork |

## Troubleshooting

- **`missing required field schema_version`** or **`name: Required`** →
  `case.yaml` needs both `schema_version: "1.1"` and `name`.
- **Claude says it can't find the files** → the workspace starts empty.
  Use a `scaffold_script` and run with `--scaffold`. The run can't read the
  eval directory or follow symlinks out of it.
- **Δ is 0 or negative** → the plugin isn't firing (check the `with-only`
  grader) or the baseline is already good at the task. Either way, the
  plugin may not be worth its always-on context cost.
- **Scores jump between runs** → that's model variance. Raise `--runs`
  before you draw conclusions.
