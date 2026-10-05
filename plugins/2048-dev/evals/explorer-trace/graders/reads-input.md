---
type: regex
pattern: "input\\.cpp"
weight: 1
---

The trace must pass through input.cpp, where the arrow key is decoded. A
reply that only says "I couldn't find Board::move" never names this file,
so it fails here.
