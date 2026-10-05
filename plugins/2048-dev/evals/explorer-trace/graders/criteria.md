---
type: llm
weight: 2
---

PASS if the answer is an ordered list that starts at `main` (main.cpp), goes
through the game loop in game.cpp and the key reading in input.cpp, and ends at
`Board::move` in board.cpp, naming a file and a function at each step.
FAIL if any of those four files is missing, the order is wrong, or the answer
invents functions that are not in the source.
