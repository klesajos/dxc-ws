#pragma once

#include <memory>

namespace g2048 {

enum class Command { None, Up, Down, Left, Right, Quit };

// Reads single keypresses from the terminal in raw mode (no Enter required).
// Supports the arrow keys and WASD; 'q' quits.
class Input {
public:
    Input();
    ~Input();

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;

    // Blocks until a key is pressed and maps it to a Command.
    Command next();

private:
    struct TerminalState;  // RAII guard that restores the terminal on exit.
    std::unique_ptr<TerminalState> state_;
};

}  // namespace g2048
