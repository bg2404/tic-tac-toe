#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace ttt {

enum class Mark : std::uint8_t { Empty, X, O };

[[nodiscard]] constexpr Mark opponent(Mark mark) noexcept {
    switch (mark) {
        case Mark::X: return Mark::O;
        case Mark::O: return Mark::X;
        case Mark::Empty: break;
    }
    return Mark::Empty;
}

[[nodiscard]] constexpr char toChar(Mark mark) noexcept {
    switch (mark) {
        case Mark::X: return 'X';
        case Mark::O: return 'O';
        case Mark::Empty: break;
    }
    return ' ';
}

enum class Outcome : std::uint8_t { InProgress, XWins, OWins, Draw };

// 3x3 board. Cells are indexed 0..8, row-major from the top-left corner.
class Board {
public:
    static constexpr std::size_t kSize = 9;
    using Line = std::array<std::size_t, 3>;

    // clang-format off
    static constexpr std::array<Line, 8> kLines{{
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},  // rows
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},  // columns
        {0, 4, 8}, {2, 4, 6},             // diagonals
    }};
    // clang-format on

    [[nodiscard]] Mark at(std::size_t cell) const { return cells_.at(cell); }
    [[nodiscard]] bool isEmpty(std::size_t cell) const {
        return cell < kSize && cells_[cell] == Mark::Empty;
    }

    // Places `mark` on `cell`. Returns false (board unchanged) if the cell is
    // out of range or occupied, or if `mark` is Mark::Empty.
    bool place(std::size_t cell, Mark mark);
    void clear(std::size_t cell) { cells_.at(cell) = Mark::Empty; }

    [[nodiscard]] std::optional<Mark> winner() const;
    [[nodiscard]] bool isFull() const;
    [[nodiscard]] Outcome outcome() const;
    [[nodiscard]] std::vector<std::size_t> emptyCells() const;

    // Side to move, assuming X always moves first.
    [[nodiscard]] Mark nextToMove() const;

    friend bool operator==(const Board&, const Board&) = default;

private:
    std::array<Mark, kSize> cells_{};
};

}  // namespace ttt
