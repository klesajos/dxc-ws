#pragma once

#include <string>

namespace g2048 {

class Board;

// Draws the board to standard output using ANSI escape codes.
class Renderer {
public:
    // Clears the screen and renders the current board state and score.
    void draw(const Board& board) const;

    // Prints a one-off message below the board (e.g. "You win!").
    void message(const std::string& text) const;
};

}  // namespace g2048
