#include "input.hpp"

#include <termios.h>
#include <unistd.h>

#include <cstdio>

namespace g2048 {

// Puts the terminal into raw mode on construction and restores the previous
// settings on destruction.
struct Input::TerminalState {
    termios previous{};

    TerminalState() {
        tcgetattr(STDIN_FILENO, &previous);
        termios raw = previous;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }

    ~TerminalState() { tcsetattr(STDIN_FILENO, TCSANOW, &previous); }
};

Input::Input() : state_(std::make_unique<TerminalState>()) {}
Input::~Input() = default;

Command Input::next() {
    const int c = std::getchar();
    switch (c) {
        case 'q':
        case 'Q':
            return Command::Quit;
        case 'w':
        case 'W':
            return Command::Up;
        case 's':
        case 'S':
            return Command::Down;
        case 'a':
        case 'A':
            return Command::Left;
        case 'd':
        case 'D':
            return Command::Right;
        case '\x1b': {  // Arrow keys arrive as ESC '[' followed by A/B/C/D.
            if (std::getchar() != '[') {
                return Command::None;
            }
            switch (std::getchar()) {
                case 'A':
                    return Command::Up;
                case 'B':
                    return Command::Down;
                case 'C':
                    return Command::Right;
                case 'D':
                    return Command::Left;
                default:
                    return Command::None;
            }
        }
        default:
            return Command::None;
    }
}

}  // namespace g2048
