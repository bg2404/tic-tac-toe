#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include "tictactoe/board.hpp"

namespace ttt {

enum class Difficulty : std::uint8_t {
    Easy,    // Plays a random empty cell.
    Medium,  // Wins if it can, otherwise blocks, otherwise random.
    Hard,    // Perfect play (minimax); never loses.
};

// Cells where placing `self` completes a line immediately.
[[nodiscard]] std::vector<std::size_t> winningMoves(const Board& board, Mark self);

// All moves with the best minimax score for `self`. Prefers faster wins and
// slower losses. Empty if the game is over.
[[nodiscard]] std::vector<std::size_t> bestMoves(const Board& board, Mark self);

// Picks a move for `self`. Precondition: the game is in progress.
[[nodiscard]] std::size_t chooseMove(const Board& board, Mark self, Difficulty difficulty,
                                     std::mt19937& rng);

}  // namespace ttt
