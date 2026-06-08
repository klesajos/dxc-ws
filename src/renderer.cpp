#include "renderer.hpp"

#include <cstdio>
#include <iostream>

#include "board.hpp"

namespace g2048 {

namespace {
constexpr int kCellWidth = 6;  // characters per tile, e.g. "  2048"
}

void Renderer::draw(const Board& board) const {
    // Clear screen and move the cursor to the top-left corner.
    std::cout << "\x1b[2J\x1b[H";
    std::cout << "  2048  —  score: " << board.score() << "\n\n";

    for (int r = 0; r < kSize; ++r) {
        for (int c = 0; c < kSize; ++c) {
            const int value = board.at(r, c);
            if (value == 0) {
                std::printf("%*s", kCellWidth, " .");
            } else {
                std::printf("%*d", kCellWidth, value);
            }
        }
        std::cout << "\n\n";
    }

    std::cout << "  Arrows / WASD to move,  q to quit\n";
    std::cout.flush();
}

void Renderer::message(const std::string& text) const {
    std::cout << "\n  " << text << "\n";
    std::cout.flush();
}

}  // namespace g2048
