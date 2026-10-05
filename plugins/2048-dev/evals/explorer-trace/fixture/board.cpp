#include "board.hpp"

namespace g2048 {

bool Board::move(Direction dir) {
    // Rotate the grid so every direction becomes "slide left", call
    // slideLineLeft() on each row, rotate back. Returns true if any tile moved.
    bool changed = false;
    // ... (slide and merge logic omitted in this fixture)
    return changed;
}

}  // namespace g2048
