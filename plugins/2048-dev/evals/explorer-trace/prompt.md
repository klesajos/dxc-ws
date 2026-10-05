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
