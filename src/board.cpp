#include "board.hpp"

#include <utility>

namespace g2048 {

Board::Board() : rng_(std::random_device{}()) {
    spawnRandom();
    spawnRandom();
}

Board::Board(Grid grid, int score)
    : grid_(grid), score_(score), rng_(std::random_device{}()) {}

int slideLineLeft(std::array<int, kSize>& line) {
    // 1) Compact: push all non-zero tiles to the front, keeping their order.
    std::array<int, kSize> out{};
    int n = 0;
    for (int value : line) {
        if (value != 0) {
            out[n++] = value;
        }
    }

    // 2) Merge equal neighbours, scanning left to right.
    int gained = 0;
    int i = 0;
    while (i + 1 < n) {
        if (out[i] == out[i + 1]) {
            out[i] *= 2;
            gained += out[i];

            // Remove the consumed tile by shifting the tail left by one.
            for (int k = i + 1; k < n - 1; ++k) {
                out[k] = out[k + 1];
            }
            out[n - 1] = 0;
            --n;
            // NOTE: index `i` is intentionally not advanced here.
        } else {
            ++i;
        }
    }

    line = out;
    return gained;
}

bool Board::move(Direction dir) {
    const Grid before = grid_;
    int gained = 0;

    // NOTE: the four branches below are near-identical. They extract a line
    // (row or column, possibly reversed), slide it left, then write it back.
    if (dir == Direction::Left) {
        for (int r = 0; r < kSize; ++r) {
            std::array<int, kSize> line = grid_[r];
            gained += slideLineLeft(line);
            grid_[r] = line;
        }
    } else if (dir == Direction::Right) {
        for (int r = 0; r < kSize; ++r) {
            std::array<int, kSize> line{};
            for (int c = 0; c < kSize; ++c) {
                line[c] = grid_[r][kSize - 1 - c];
            }
            gained += slideLineLeft(line);
            for (int c = 0; c < kSize; ++c) {
                grid_[r][kSize - 1 - c] = line[c];
            }
        }
    } else if (dir == Direction::Up) {
        for (int c = 0; c < kSize; ++c) {
            std::array<int, kSize> line{};
            for (int r = 0; r < kSize; ++r) {
                line[r] = grid_[r][c];
            }
            gained += slideLineLeft(line);
            for (int r = 0; r < kSize; ++r) {
                grid_[r][c] = line[r];
            }
        }
    } else {  // Direction::Down
        for (int c = 0; c < kSize; ++c) {
            std::array<int, kSize> line{};
            for (int r = 0; r < kSize; ++r) {
                line[r] = grid_[kSize - 1 - r][c];
            }
            gained += slideLineLeft(line);
            for (int r = 0; r < kSize; ++r) {
                grid_[kSize - 1 - r][c] = line[r];
            }
        }
    }

    score_ += gained;
    return grid_ != before;
}

bool Board::spawnRandom() {
    std::array<std::pair<int, int>, kSize * kSize> empties{};
    int n = 0;
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            if (grid_[r][c] == 0) {
                empties[n++] = {r, c};
            }
        }
    }
    if (n == 0) {
        return false;
    }

    std::uniform_int_distribution<int> pickCell(0, n - 1);
    const auto [r, c] = empties[pickCell(rng_)];

    std::uniform_int_distribution<int> roll(1, 10);
    grid_[r][c] = (roll(rng_) == 1) ? 4 : 2;
    return true;
}

bool Board::isGameOver() const {
    // A board is "over" only when no move is possible. For now we just look
    // for an empty cell.
    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            if (grid_[r][c] == 0) {
                return false;
            }
        }
    }
    return true;
}

bool Board::hasWon() const {
    for (const auto& row : grid_) {
        for (int value : row) {
            if (value >= kWinValue) {
                return true;
            }
        }
    }
    return false;
}

}  // namespace g2048
