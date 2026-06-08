#pragma once

#include <array>
#include <random>

namespace g2048 {

// Side length of the (square) board.
inline constexpr int kSize = 4;

using Grid = std::array<std::array<int, kSize>, kSize>;

enum class Direction { Up, Down, Left, Right };

// Pure game state + rules for 2048. No I/O lives here, which is what makes
// the class straightforward to unit-test.
class Board {
public:
    // Starts an empty board and seeds it with two random tiles.
    Board();

    // Builds a board from an explicit grid (handy for tests). No tiles are
    // spawned, so the state is fully deterministic.
    explicit Board(Grid grid, int score = 0);

    // Applies a move in the given direction.
    // Returns true if the board actually changed.
    bool move(Direction dir);

    // Spawns a single tile (2 with 90% probability, otherwise 4) in a random
    // empty cell. Returns false if the board was already full.
    bool spawnRandom();

    bool isGameOver() const;
    bool hasWon() const;  // true once any tile reaches kWinValue

    int score() const { return score_; }
    const Grid& grid() const { return grid_; }
    int at(int row, int col) const { return grid_[row][col]; }

    static constexpr int kWinValue = 2048;

private:
    Grid grid_{};
    int score_ = 0;
    std::mt19937 rng_;
};

// Slides and merges a single line towards index 0 (the "left").
// Returns the score gained from merges this move.
// Exposed as a free function so it can be unit-tested in isolation.
int slideLineLeft(std::array<int, kSize>& line);

}  // namespace g2048
