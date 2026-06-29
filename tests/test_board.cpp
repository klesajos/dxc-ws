#include <catch2/catch_test_macros.hpp>

#include "board.hpp"

using namespace g2048;

namespace {

// Convenience: run slideLineLeft on a brace-initialised line and return it.
std::array<int, kSize> slid(std::array<int, kSize> line) {
    slideLineLeft(line);
    return line;
}

}  // namespace

// ---------------------------------------------------------------------------
// slideLineLeft — the heart of the game.
// ---------------------------------------------------------------------------

TEST_CASE("empty line stays empty") {
    REQUIRE(slid({0, 0, 0, 0}) == std::array<int, kSize>{0, 0, 0, 0});
}

TEST_CASE("tiles compact towards the left without merging") {
    REQUIRE(slid({0, 2, 0, 4}) == std::array<int, kSize>{2, 4, 0, 0});
    REQUIRE(slid({2, 4, 8, 16}) == std::array<int, kSize>{2, 4, 8, 16});
}

TEST_CASE("a single pair merges") {
    std::array<int, kSize> line{2, 2, 0, 0};
    const int gained = slideLineLeft(line);
    REQUIRE(line == std::array<int, kSize>{4, 0, 0, 0});
    REQUIRE(gained == 4);
}

TEST_CASE("two separate pairs merge into two tiles") {
    std::array<int, kSize> line{2, 2, 2, 2};
    const int gained = slideLineLeft(line);
    REQUIRE(line == std::array<int, kSize>{4, 4, 0, 0});
    REQUIRE(gained == 8);
}

TEST_CASE("only the matching neighbours merge") {
    REQUIRE(slid({4, 2, 2, 0}) == std::array<int, kSize>{4, 4, 0, 0});
}

// TODO (participants): add a test for the line {4, 4, 8, 0}.
//   What *should* the result be? Run it and see what actually happens.

// ---------------------------------------------------------------------------
// Board::move — directions.
// ---------------------------------------------------------------------------

TEST_CASE("move left collapses each row") {
    Board board(Grid{{{2, 2, 0, 0}, {0, 4, 4, 0}, {0, 0, 0, 0}, {8, 0, 8, 0}}});
    const bool changed = board.move(Direction::Left);
    REQUIRE(changed);
    REQUIRE(board.grid() ==
            Grid{{{4, 0, 0, 0}, {8, 0, 0, 0}, {0, 0, 0, 0}, {16, 0, 0, 0}}});
}

TEST_CASE("move right collapses towards the right edge") {
    Board board(Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    board.move(Direction::Right);
    REQUIRE(board.at(0, 3) == 4);
}

TEST_CASE("move up collapses columns") {
    Board board(Grid{{{2, 0, 0, 0}, {2, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    board.move(Direction::Up);
    REQUIRE(board.at(0, 0) == 4);
}

TEST_CASE("a move that changes nothing returns false") {
    Board board(Grid{{{2, 4, 2, 4}, {4, 2, 4, 2}, {2, 4, 2, 4}, {4, 2, 4, 2}}});
    REQUIRE_FALSE(board.move(Direction::Left));
}

TEST_CASE("score accumulates across moves") {
    Board board(Grid{{{2, 2, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    board.move(Direction::Left);  // +4
    REQUIRE(board.score() == 4);
}

// ---------------------------------------------------------------------------
// Win / lose detection.
// ---------------------------------------------------------------------------

TEST_CASE("hasWon triggers at 2048") {
    Board board(Grid{{{2048, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}});
    REQUIRE(board.hasWon());
}

TEST_CASE("a board with an empty cell is not game over") {
    Board board(Grid{{{2, 4, 2, 4}, {4, 2, 4, 2}, {2, 4, 2, 4}, {4, 2, 4, 0}}});
    REQUIRE_FALSE(board.isGameOver());
}

TEST_CASE("a fully locked board is game over") {
    Board board(Grid{{{2, 4, 2, 4}, {4, 2, 4, 2}, {2, 4, 2, 4}, {4, 2, 4, 2}}});
    REQUIRE(board.isGameOver());
}

// TODO (participants): a *full* board can still be playable if it has two
//   equal neighbours. Write a test for a full board that contains a mergeable
//   pair and assert that isGameOver() returns false.
