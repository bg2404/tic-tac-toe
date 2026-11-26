#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <optional>
#include <random>
#include <string>

#include "tictactoe/board.hpp"
#include "tictactoe/game.hpp"
#include "tictactoe/terminal.hpp"

namespace ttt {

struct Score {
    int wins = 0;
    int losses = 0;
    int draws = 0;
};

enum class Tone : std::uint8_t { Normal, Warning, Good, Bad };

struct MenuView {
    GameConfig config;
    int selectedField = 0;  // 0 = difficulty, 1 = side
    Score score;
};

struct GameView {
    Board board;
    GameConfig config;
    std::size_t cursor = 4;
    bool showCursor = true;
    std::optional<std::size_t> lastComputerMove;
    Score score;
    std::string status;
    Tone tone = Tone::Normal;
    std::string hints;
};

// Pure renderers: return the frame text (lines separated by '\n') including
// ANSI style codes. With `color` false, only bold/dim/reverse/underline are used.
[[nodiscard]] std::string renderMenu(const MenuView& view, bool color);
[[nodiscard]] std::string renderGame(const GameView& view, bool color);

// Full-screen, keyboard-driven interface. Input arrives through `readKey`,
// so it can be scripted in tests.
class Tui {
public:
    using KeyReader = std::function<term::Key()>;

    struct Options {
        bool color = true;
        std::chrono::milliseconds computerDelay{350};
    };

    Tui(KeyReader readKey, std::ostream& out, std::mt19937 rng, Options options);

    void run();

    [[nodiscard]] const Score& score() const noexcept { return score_; }

private:
    enum class Next : std::uint8_t { Menu, Play, Quit };

    Next menu();
    Next playRound();
    void draw(const std::string& frame);

    KeyReader readKey_;
    std::ostream& out_;
    std::mt19937 rng_;
    Options options_;
    GameConfig config_{.difficulty = Difficulty::Medium, .human = Mark::X};
    Score score_;
};

}  // namespace ttt
