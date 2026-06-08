#pragma once

#include "board.hpp"
#include "input.hpp"
#include "renderer.hpp"

namespace g2048 {

// Wires the board, renderer and input together and runs the main loop.
class Game {
public:
    void run();

private:
    Board board_;
    Renderer renderer_;
    Input input_;
};

// Maps a movement Command to a board Direction. Caller must ensure `cmd` is
// one of the four movement commands.
Direction toDirection(Command cmd);

}  // namespace g2048
