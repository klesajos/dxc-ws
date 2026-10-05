#include "input.hpp"

#include <cstdio>

namespace g2048 {

Command Input::next() {
    const int c = std::getchar();
    switch (c) {
        case 'q':
            return Command::Quit;
        case 'a':
            return Command::Left;
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
