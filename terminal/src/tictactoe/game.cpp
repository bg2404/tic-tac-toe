#include "tictactoe/game.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <istream>
#include <ostream>
#include <string_view>

namespace ttt {
namespace {

std::string_view trim(std::string_view s) {
    constexpr std::string_view kWhitespace = " \t\r\n";
    const auto first = s.find_first_not_of(kWhitespace);
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = s.find_last_not_of(kWhitespace);
    return s.substr(first, last - first + 1);
}

bool isQuitCommand(std::string_view s) {
    std::string lower(s);
    std::ranges::transform(lower, lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower == "q" || lower == "quit" || lower == "exit";
}

std::optional<int> parseInt(std::string_view s) {
    int value = 0;
    const auto* const end = s.data() + s.size();
    const auto [ptr, ec] = std::from_chars(s.data(), end, value);
    if (ec != std::errc{} || ptr != end) {
        return std::nullopt;
    }
    return value;
}

}  // namespace

std::string render(const Board& board) {
    std::string result;
    for (std::size_t row = 0; row < 3; ++row) {
        result += ' ';
        for (std::size_t col = 0; col < 3; ++col) {
            const std::size_t cell = row * 3 + col;
            const Mark mark = board.at(cell);
            result += mark == Mark::Empty ? static_cast<char>('1' + cell) : toChar(mark);
            if (col < 2) {
                result += " | ";
            }
        }
        result += '\n';
        if (row < 2) {
            result += "---+---+---\n";
        }
    }
    return result;
}

Session::Session(std::istream& in, std::ostream& out, std::mt19937 rng)
    : in_(in), out_(out), rng_(rng) {}

void Session::run() {
    out_ << "TIC-TAC-TOE\n"
         << "Type 'q' at any prompt to quit.\n\n";

    while (const auto config = promptConfig()) {
        const auto outcome = play(*config);
        if (!outcome) {
            break;
        }
        announce(*outcome, *config);

        const auto again = readLine("Play again? [y/N]: ");
        if (!again || again->empty() ||
            std::tolower(static_cast<unsigned char>((*again)[0])) != 'y') {
            break;
        }
        out_ << '\n';
    }
    out_ << "Goodbye.\n";
}

std::optional<GameConfig> Session::promptConfig() {
    out_ << "Choose difficulty:\n"
         << "  1. Easy\n"
         << "  2. Medium\n"
         << "  3. Hard\n";
    const auto level = promptChoice("Difficulty [1-3]: ", 1, 3);
    if (!level) {
        return std::nullopt;
    }

    out_ << "Choose your side:\n"
         << "  1. Player 1 (X, moves first)\n"
         << "  2. Player 2 (O, moves second)\n";
    const auto side = promptChoice("Player [1-2]: ", 1, 2);
    if (!side) {
        return std::nullopt;
    }

    return GameConfig{
        .difficulty = static_cast<Difficulty>(*level - 1),
        .human = *side == 1 ? Mark::X : Mark::O,
    };
}

std::optional<Outcome> Session::play(const GameConfig& config) {
    Board board;
    const Mark computer = opponent(config.human);
    out_ << "\nLevel: " << difficultyName(config.difficulty) << " | You are "
         << toChar(config.human) << "\n\n";

    while (board.outcome() == Outcome::InProgress) {
        const Mark toMove = board.nextToMove();
        if (toMove == config.human) {
            out_ << render(board) << '\n';
            const auto cell = promptHumanMove(board);
            if (!cell) {
                return std::nullopt;
            }
            board.place(*cell, toMove);
        } else {
            const auto cell = chooseMove(board, computer, config.difficulty, rng_);
            board.place(cell, toMove);
            out_ << "Computer marked " << (cell + 1) << ".\n";
        }
    }

    out_ << '\n' << render(board) << '\n';
    return board.outcome();
}

std::optional<std::string> Session::readLine(const std::string& prompt) {
    out_ << prompt << std::flush;
    std::string line;
    if (!std::getline(in_, line)) {
        out_ << '\n';
        return std::nullopt;
    }
    const auto trimmed = trim(line);
    if (isQuitCommand(trimmed)) {
        return std::nullopt;
    }
    return std::string(trimmed);
}

std::optional<int> Session::promptChoice(const std::string& prompt, int min, int max) {
    while (true) {
        const auto line = readLine(prompt);
        if (!line) {
            return std::nullopt;
        }
        if (const auto value = parseInt(*line); value && *value >= min && *value <= max) {
            return value;
        }
        out_ << "Invalid input. Enter a number from " << min << " to " << max << ".\n";
    }
}

std::optional<std::size_t> Session::promptHumanMove(const Board& board) {
    while (true) {
        const auto choice = promptChoice("Your move [1-9]: ", 1, 9);
        if (!choice) {
            return std::nullopt;
        }
        const auto cell = static_cast<std::size_t>(*choice - 1);
        if (board.isEmpty(cell)) {
            return cell;
        }
        out_ << "Cell " << *choice << " is already marked by "
             << (board.at(cell) == opponent(board.nextToMove()) ? "the computer" : "you")
             << ". Choose another.\n";
    }
}

void Session::announce(Outcome outcome, const GameConfig& config) {
    switch (outcome) {
        case Outcome::Draw: out_ << "Draw game.\n"; return;
        case Outcome::XWins:
        case Outcome::OWins: {
            const Mark winner = outcome == Outcome::XWins ? Mark::X : Mark::O;
            out_ << (winner == config.human ? "You win!\n" : "Computer wins.\n");
            return;
        }
        case Outcome::InProgress: return;
    }
}

}  // namespace ttt
