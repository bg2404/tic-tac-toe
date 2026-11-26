#pragma once

#include <iosfwd>
#include <optional>
#include <random>
#include <string>

#include "tictactoe/ai.hpp"
#include "tictactoe/board.hpp"

namespace ttt {

struct GameConfig {
    Difficulty difficulty = Difficulty::Hard;
    Mark human = Mark::X;  // X moves first.
};

// Text rendering of the board; empty cells show their 1-based number.
[[nodiscard]] std::string render(const Board& board);

// Interactive console session. All I/O goes through the supplied streams so
// the session can be driven from tests.
class Session {
public:
    Session(std::istream& in, std::ostream& out, std::mt19937 rng);

    // Runs menus and games until the user quits or input ends.
    void run();

    // Asks for difficulty and side. Returns nullopt if the user quits.
    [[nodiscard]] std::optional<GameConfig> promptConfig();

    // Plays one game. Returns nullopt if the user quits mid-game.
    [[nodiscard]] std::optional<Outcome> play(const GameConfig& config);

private:
    // Reads one trimmed line. Returns nullopt on EOF or a quit command.
    [[nodiscard]] std::optional<std::string> readLine(const std::string& prompt);
    [[nodiscard]] std::optional<int> promptChoice(const std::string& prompt, int min, int max);
    [[nodiscard]] std::optional<std::size_t> promptHumanMove(const Board& board);
    void announce(Outcome outcome, const GameConfig& config);

    std::istream& in_;
    std::ostream& out_;
    std::mt19937 rng_;
};

}  // namespace ttt
