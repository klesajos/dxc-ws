// test-coverage-audit — a read-only, deterministic, re-runnable audit.
//
// It fans out one read-only reader over each source file to inventory the
// behaviours a unit test could target, maps the existing Catch2 tests onto
// that inventory, then reduces the two into a prioritized gap report.
//
// Every agent runs as the built-in read-only `Explore` agent type, so the whole
// workflow CANNOT edit, write, or build anything — it only reads and reports.
// That is what makes it safe to run repeatedly: same repo in, same report out.
//
// Run it from Claude Code with:  /test-coverage-audit

export const meta = {
  name: 'test-coverage-audit',
  description:
    'Audit src/ test coverage against tests/test_board.cpp and emit a prioritized gap report.',
  phases: [
    { title: 'Inventory', detail: 'one read-only reader per src file enumerates behaviours' },
    { title: 'Map tests', detail: 'map each TEST_CASE onto the behaviour inventory' },
    { title: 'Report gaps', detail: 'cross-reference and emit a prioritized gap report' },
  ],
}

// The source files to inventory, with a hint of what each contains so the
// reader agent orients quickly. Game rules (board.cpp) are the testable core.
const SRC_FILES = [
  { file: 'src/board.cpp', note: 'pure rules: slideLineLeft, move, spawnRandom, isGameOver, hasWon' },
  { file: 'src/game.cpp', note: 'main loop: toDirection, Game::run, spawn + redraw, win/over checks' },
  { file: 'src/input.cpp', note: 'raw-mode terminal: getchar -> Command (WASD + arrow keys)' },
  { file: 'src/renderer.cpp', note: 'draws the grid + score to the terminal' },
  { file: 'src/main.cpp', note: 'entry point: constructs Game, calls run()' },
]

// ── Structured-output schemas ──────────────────────────────────────────────
// Each agent is forced to return data matching its schema, so the script never
// has to parse free text.

// BehaviorInventory: what one source file does, branch by branch.
const BEHAVIOR_SCHEMA = {
  type: 'object',
  properties: {
    behaviors: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          id: { type: 'string', description: 'stable dotted id, e.g. board.move.merge-row' },
          file: { type: 'string' },
          line: { type: 'number', description: 'line where the behaviour lives' },
          behavior: { type: 'string', description: 'one-line description of what happens' },
          pureLogic: { type: 'boolean', description: 'true if free of I/O and unit-testable' },
          sideEffects: { type: 'string', description: 'I/O or state mutation, or "none"' },
        },
        required: ['id', 'file', 'line', 'behavior', 'pureLogic'],
      },
    },
  },
  required: ['behaviors'],
}

// CoverageMap: which behaviour ids each existing TEST_CASE exercises.
const COVERAGE_SCHEMA = {
  type: 'object',
  properties: {
    coverage: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          testCase: { type: 'string', description: 'the TEST_CASE name' },
          line: { type: 'number' },
          coversBehaviorIds: { type: 'array', items: { type: 'string' } },
        },
        required: ['testCase', 'coversBehaviorIds'],
      },
    },
  },
  required: ['coverage'],
}

// GapReport: behaviours no test covers, prioritized, plus a rendered table.
const GAP_SCHEMA = {
  type: 'object',
  properties: {
    gaps: {
      type: 'array',
      items: {
        type: 'object',
        properties: {
          behaviorId: { type: 'string' },
          file: { type: 'string' },
          line: { type: 'number' },
          why: { type: 'string', description: 'why this gap matters' },
          suggestedTestName: { type: 'string', description: 'a plain-English TEST_CASE name' },
          priority: { type: 'string', enum: ['high', 'medium', 'low'] },
        },
        required: ['behaviorId', 'file', 'why', 'suggestedTestName', 'priority'],
      },
    },
    markdown: { type: 'string', description: 'the full rendered gap report as Markdown' },
  },
  required: ['gaps', 'markdown'],
}

// ── Phase 1: inventory each source file in parallel ────────────────────────
// A barrier (parallel) is correct here: Phase 2 needs the WHOLE inventory
// before it can map tests onto it. (Contrast pipeline(), which suits per-item
// independent chains — see the feature-pipeline described in the guide.)
phase('Inventory')
const inventories = await parallel(
  SRC_FILES.map((f) => () =>
    agent(
      `Read ${f.file} in this 2048 C++ repo (${f.note}). Enumerate every distinct ` +
        `behaviour or branch that a unit test could target, in source order. For each, give ` +
        `a stable dotted id, the file, the line it lives on, a one-line description, whether ` +
        `it is pure logic (free of I/O, so unit-testable), and its side effects. Do not ` +
        `propose tests yet — only inventory what the code actually does.`,
      { label: `inventory:${f.file}`, phase: 'Inventory', schema: BEHAVIOR_SCHEMA, agentType: 'Explore' }
    )
  )
)
const behaviors = inventories.filter(Boolean).flatMap((r) => r.behaviors)

// ── Phase 2: map the existing tests onto the inventory ─────────────────────
phase('Map tests')
const coverageResult = await agent(
  `Read tests/test_board.cpp in this 2048 C++ repo. For each Catch2 TEST_CASE, list which ` +
    `of the following known behaviours it exercises, by id (empty array if it matches none). ` +
    `Behaviours:\n${JSON.stringify(behaviors, null, 2)}`,
  { label: 'map:tests', phase: 'Map tests', schema: COVERAGE_SCHEMA, agentType: 'Explore' }
)

// ── Phase 3: reduce into a prioritized gap report ──────────────────────────
phase('Report gaps')
const report = await agent(
  `You are auditing unit-test coverage for a 2048 C++ game. Cross-reference the behaviour ` +
    `inventory against the coverage map and produce a PRIORITIZED gap report. A gap is a ` +
    `behaviour with pureLogic=true that no TEST_CASE covers; prioritize by risk and how core ` +
    `the behaviour is (slide/merge logic is the heart of the game).\n\n` +
    `Known high-value gaps you should expect to surface:\n` +
    `- slideLineLeft on {4, 4, 8, 0} (the TODO in tests/test_board.cpp).\n` +
    `- spawnRandom: returns false on a full board, and its 2-vs-4 distribution.\n` +
    `- a FULL board that still contains a mergeable pair: isGameOver() should return false. ` +
    `Note that isGameOver() in src/board.cpp only checks for empty cells, so a correct test ` +
    `for this case currently FAILS — flag it as a latent bug, do not hide it.\n` +
    `- multi-move sequences (score/state across several moves).\n` +
    `- the 'changed' flag in src/game.cpp is computed but ignored, so a tile spawns even ` +
    `after a no-op move (game-loop behaviour, currently untested).\n\n` +
    `BEHAVIOURS:\n${JSON.stringify(behaviors, null, 2)}\n\n` +
    `COVERAGE:\n${JSON.stringify(coverageResult.coverage, null, 2)}\n\n` +
    `Return a 'gaps' array and a 'markdown' field. The markdown must contain a table with ` +
    `columns: behaviour id | file:line | priority | suggested TEST_CASE name | why. This is a ` +
    `read-only audit: recommend tests to add, do not edit anything.`,
  { label: 'reduce:gaps', phase: 'Report gaps', schema: GAP_SCHEMA, agentType: 'Explore' }
)

log(`Audited ${behaviors.length} behaviours; found ${report.gaps.length} coverage gaps.`)
return report.markdown
