#include "tictactoe/board.hpp"

#include <algorithm>

namespace ttt {

bool Board::place(std::size_t cell, Mark mark) {
    if (mark == Mark::Empty || !isEmpty(cell)) {
        return false;
    }
    cells_[cell] = mark;
    return true;
}

std::optional<Board::Line> Board::winningLine() const {
    for (const auto& line : kLines) {
        const auto [a, b, c] = line;
        if (cells_[a] != Mark::Empty && cells_[a] == cells_[b] && cells_[b] == cells_[c]) {
            return line;
        }
    }
    return std::nullopt;
}

std::optional<Mark> Board::winner() const {
    if (const auto line = winningLine()) {
        return cells_[(*line)[0]];
    }
    return std::nullopt;
}

bool Board::isFull() const {
    return std::ranges::none_of(cells_, [](Mark m) { return m == Mark::Empty; });
}

Outcome Board::outcome() const {
    if (const auto w = winner()) {
        return *w == Mark::X ? Outcome::XWins : Outcome::OWins;
    }
    return isFull() ? Outcome::Draw : Outcome::InProgress;
}

std::vector<std::size_t> Board::emptyCells() const {
    std::vector<std::size_t> result;
    result.reserve(kSize);
    for (std::size_t i = 0; i < kSize; ++i) {
        if (cells_[i] == Mark::Empty) {
            result.push_back(i);
        }
    }
    return result;
}

Mark Board::nextToMove() const {
    const auto xs = std::ranges::count(cells_, Mark::X);
    const auto os = std::ranges::count(cells_, Mark::O);
    return xs <= os ? Mark::X : Mark::O;
}

}  // namespace ttt
