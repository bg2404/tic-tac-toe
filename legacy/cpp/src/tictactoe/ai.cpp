#include "tictactoe/ai.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>

namespace ttt {
namespace {

std::size_t pickRandom(const std::vector<std::size_t>& cells, std::mt19937& rng) {
    if (cells.empty()) {
        throw std::logic_error("no move available");
    }
    std::uniform_int_distribution<std::size_t> dist(0, cells.size() - 1);
    return cells[dist(rng)];
}

// Score from the perspective of `self`; `toMove` is the side about to play.
// Win = +10 - depth, loss = depth - 10, draw = 0.
int minimax(Board& board, Mark self, Mark toMove, int depth) {
    if (const auto w = board.winner()) {
        return *w == self ? 10 - depth : depth - 10;
    }
    if (board.isFull()) {
        return 0;
    }

    const bool maximizing = toMove == self;
    int best = maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();
    for (const auto cell : board.emptyCells()) {
        board.place(cell, toMove);
        const int score = minimax(board, self, opponent(toMove), depth + 1);
        board.clear(cell);
        best = maximizing ? std::max(best, score) : std::min(best, score);
    }
    return best;
}

}  // namespace

std::vector<std::size_t> winningMoves(const Board& board, Mark self) {
    std::vector<std::size_t> result;
    for (const auto cell : board.emptyCells()) {
        Board copy = board;
        copy.place(cell, self);
        if (copy.winner() == self) {
            result.push_back(cell);
        }
    }
    return result;
}

std::vector<std::size_t> bestMoves(const Board& board, Mark self) {
    std::vector<std::size_t> result;
    if (board.outcome() != Outcome::InProgress) {
        return result;
    }

    Board scratch = board;
    int best = std::numeric_limits<int>::min();
    for (const auto cell : board.emptyCells()) {
        scratch.place(cell, self);
        const int score = minimax(scratch, self, opponent(self), 1);
        scratch.clear(cell);
        if (score > best) {
            best = score;
            result.clear();
        }
        if (score == best) {
            result.push_back(cell);
        }
    }
    return result;
}

std::size_t chooseMove(const Board& board, Mark self, Difficulty difficulty, std::mt19937& rng) {
    assert(board.outcome() == Outcome::InProgress);

    switch (difficulty) {
        case Difficulty::Easy: break;
        case Difficulty::Medium:
            if (const auto wins = winningMoves(board, self); !wins.empty()) {
                return pickRandom(wins, rng);
            }
            if (const auto blocks = winningMoves(board, opponent(self)); !blocks.empty()) {
                return pickRandom(blocks, rng);
            }
            break;
        case Difficulty::Hard: return pickRandom(bestMoves(board, self), rng);
    }
    return pickRandom(board.emptyCells(), rng);
}

}  // namespace ttt
