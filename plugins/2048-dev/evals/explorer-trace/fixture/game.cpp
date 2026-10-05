#include "game.hpp"

namespace g2048 {

Direction toDirection(Command cmd) {
    switch (cmd) {
        case Command::Up:
            return Direction::Up;
        case Command::Down:
            return Direction::Down;
        case Command::Left:
            return Direction::Left;
        case Command::Right:
        default:
            return Direction::Right;
    }
}

void Game::run() {
    renderer_.draw(board_);

    while (true) {
        const Command cmd = input_.next();
        if (cmd == Command::Quit) {
            break;
        }
        if (cmd == Command::None) {
            continue;
        }

        const bool changed = board_.move(toDirection(cmd));

        board_.spawnRandom();
        renderer_.draw(board_);
    }
}

}  // namespace g2048
