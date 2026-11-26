#include "tictactoe/tui.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <ostream>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "tictactoe/ai.hpp"

namespace ttt {
namespace {

// SGR (Select Graphic Rendition) parameters.
namespace sgr {
constexpr int kBold = 1;
constexpr int kDim = 2;
constexpr int kUnderline = 4;
constexpr int kReverse = 7;
constexpr int kFgBlack = 30;
constexpr int kFgRed = 31;
constexpr int kFgGreen = 32;
constexpr int kFgYellow = 33;
constexpr int kFgBrightMagenta = 95;
constexpr int kFgBrightCyan = 96;
constexpr int kFgBrightWhite = 97;
constexpr int kBgRed = 41;
constexpr int kBgGreen = 42;
}  // namespace sgr

using Sgr = std::vector<int>;

constexpr bool isColorCode(int code) {
    return (code >= 30 && code <= 49) || (code >= 90 && code <= 107);
}

Sgr operator+(Sgr a, const Sgr& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

class Painter {
public:
    explicit Painter(bool color) : color_(color) {}

    [[nodiscard]] bool color() const { return color_; }

    [[nodiscard]] std::string operator()(std::string_view text, const Sgr& codes) const {
        std::string params;
        for (const int code : codes) {
            if (!color_ && isColorCode(code)) {
                continue;
            }
            if (!params.empty()) {
                params += ';';
            }
            params += std::to_string(code);
        }
        if (params.empty()) {
            return std::string(text);
        }
        return "\x1b[" + params + 'm' + std::string(text) + "\x1b[0m";
    }

private:
    bool color_;
};

constexpr std::string_view kTitle = "T I C · T A C · T O E";

Sgr markStyle(Mark mark) {
    return mark == Mark::X ? Sgr{sgr::kBold, sgr::kFgBrightCyan}
                           : Sgr{sgr::kBold, sgr::kFgBrightMagenta};
}

Sgr toneStyle(Tone tone) {
    switch (tone) {
        case Tone::Warning: return {sgr::kFgYellow};
        case Tone::Good: return {sgr::kBold, sgr::kFgGreen};
        case Tone::Bad: return {sgr::kBold, sgr::kFgRed};
        case Tone::Normal: break;
    }
    return {};
}

std::string_view difficultyBlurb(Difficulty d) {
    switch (d) {
        case Difficulty::Easy: return "Plays random moves.";
        case Difficulty::Medium: return "Takes a win or blocks yours; otherwise random.";
        case Difficulty::Hard: return "Perfect play. The best you can do is draw.";
    }
    return "";
}

std::string padRight(std::string_view s, std::size_t width) {
    std::string result(s);
    if (result.size() < width) {
        result.append(width - result.size(), ' ');
    }
    return result;
}

std::string joinLines(const std::vector<std::string>& lines) {
    std::string result;
    for (const auto& line : lines) {
        result += line;
        result += '\n';
    }
    return result;
}

// Input actions, independent of which physical key produced them.
enum class Action : std::uint8_t { None, Up, Down, Left, Right, Confirm, Back, Quit, Digit };

struct Input {
    Action action = Action::None;
    int digit = 0;
};

Input toInput(const term::Key& key) {
    using term::KeyType;
    switch (key.type) {
        case KeyType::Up: return {Action::Up};
        case KeyType::Down: return {Action::Down};
        case KeyType::Left: return {Action::Left};
        case KeyType::Right: return {Action::Right};
        case KeyType::Enter:
        case KeyType::Space: return {Action::Confirm};
        case KeyType::Escape:
        case KeyType::Backspace: return {Action::Back};
        case KeyType::Interrupt:
        case KeyType::Eof: return {Action::Quit};
        case KeyType::Char: break;
        case KeyType::None: return {};
    }
    const char c = static_cast<char>(std::tolower(static_cast<unsigned char>(key.ch)));
    if (c >= '1' && c <= '9') {
        return {Action::Digit, c - '0'};
    }
    switch (c) {
        case 'w':
        case 'k': return {Action::Up};
        case 's':
        case 'j': return {Action::Down};
        case 'a':
        case 'h': return {Action::Left};
        case 'd':
        case 'l': return {Action::Right};
        case 'r': return {Action::Confirm};
        case 'm': return {Action::Back};
        case 'q': return {Action::Quit};
        default: return {};
    }
}

std::size_t moveCursor(std::size_t cursor, Action action) {
    std::size_t row = cursor / 3;
    std::size_t col = cursor % 3;
    switch (action) {
        case Action::Up: row = (row + 2) % 3; break;
        case Action::Down: row = (row + 1) % 3; break;
        case Action::Left: col = (col + 2) % 3; break;
        case Action::Right: col = (col + 1) % 3; break;
        default: break;
    }
    return row * 3 + col;
}

}  // namespace

std::string renderMenu(const MenuView& view, bool color) {
    const Painter paint(color);
    const std::array<std::string, 2> labels{"Difficulty", "Play as"};
    const std::array<std::string, 2> values{
        std::string(difficultyName(view.config.difficulty)),
        view.config.human == Mark::X ? "X  (moves first)" : "O  (moves second)",
    };

    std::vector<std::string> lines{"", "  " + paint(kTitle, {sgr::kBold}), ""};
    for (std::size_t i = 0; i < labels.size(); ++i) {
        const bool selected = static_cast<int>(i) == view.selectedField;
        std::string line = "  ";
        line += selected ? paint("▸", {sgr::kBold, sgr::kFgBrightCyan}) : " ";
        line += ' ';
        line += paint(padRight(labels[i], 12), selected ? Sgr{sgr::kBold} : Sgr{});
        if (selected) {
            line += paint("‹ ", {sgr::kDim}) + paint(values[i], {sgr::kBold, sgr::kReverse}) +
                    paint(" ›", {sgr::kDim});
        } else {
            line += "  " + values[i];
        }
        lines.push_back(std::move(line));
    }
    lines.emplace_back();
    lines.push_back("    " + paint(difficultyBlurb(view.config.difficulty), {sgr::kDim}));
    lines.emplace_back();
    const Score& s = view.score;
    if (s.wins + s.losses + s.draws > 0) {
        lines.push_back("    Score  You " + std::to_string(s.wins) + " · Computer " +
                        std::to_string(s.losses) + " · Draws " + std::to_string(s.draws));
        lines.emplace_back();
    }
    lines.push_back("  " + paint("↑↓ select · ←→ change · Enter start · q quit", {sgr::kDim}));
    return joinLines(lines);
}

std::string renderGame(const GameView& view, bool color) {
    const Painter paint(color);
    const Board& board = view.board;
    const auto winLine = board.winningLine();
    const bool humanWon = board.winner() == view.config.human;
    const auto inWinLine = [&](std::size_t cell) {
        return winLine && std::ranges::find(*winLine, cell) != winLine->end();
    };

    std::vector<std::string> boardLines;
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t sub = 0; sub < 3; ++sub) {
            std::string line;
            for (std::size_t col = 0; col < 3; ++col) {
                const std::size_t cell = row * 3 + col;
                Sgr bg;
                if (inWinLine(cell)) {
                    bg = !color     ? Sgr{sgr::kBold, sgr::kReverse}
                         : humanWon ? Sgr{sgr::kBold, sgr::kBgGreen, sgr::kFgBlack}
                                    : Sgr{sgr::kBold, sgr::kBgRed, sgr::kFgBrightWhite};
                } else if (view.showCursor && view.cursor == cell) {
                    bg = {sgr::kReverse};
                }

                if (sub == 1) {
                    const Mark mark = board.at(cell);
                    const char glyph =
                        mark == Mark::Empty ? static_cast<char>('1' + cell) : toChar(mark);
                    Sgr fg = mark == Mark::Empty ? Sgr{sgr::kDim} : markStyle(mark);
                    if (view.lastComputerMove == cell) {
                        fg.push_back(sgr::kUnderline);
                    }
                    line +=
                        paint("   ", bg) + paint(std::string(1, glyph), fg + bg) + paint("   ", bg);
                } else {
                    line += paint("       ", bg);
                }
                if (col < 2) {
                    line += paint("│", {sgr::kDim});
                }
            }
            boardLines.push_back(std::move(line));
        }
        if (row < 2) {
            boardLines.push_back(paint("───────┼───────┼───────", {sgr::kDim}));
        }
    }

    const Score& s = view.score;
    std::vector<std::string> panel(boardLines.size());
    panel[1] =
        paint("Level     ", {sgr::kDim}) + std::string(difficultyName(view.config.difficulty));
    panel[2] = paint("You are   ", {sgr::kDim}) +
               paint(std::string(1, toChar(view.config.human)), markStyle(view.config.human)) +
               paint(view.config.human == Mark::X ? "  (first)" : "  (second)", {sgr::kDim});
    panel[5] = paint("Score", {sgr::kBold});
    panel[6] = "You       " + std::to_string(s.wins);
    panel[7] = "Computer  " + std::to_string(s.losses);
    panel[8] = "Draws     " + std::to_string(s.draws);

    std::vector<std::string> lines{"", "  " + paint(kTitle, {sgr::kBold}), ""};
    for (std::size_t i = 0; i < boardLines.size(); ++i) {
        lines.push_back("  " + boardLines[i] + "     " + panel[i]);
    }
    lines.emplace_back();
    lines.push_back("  " + paint(view.status, toneStyle(view.tone)));
    lines.emplace_back();
    lines.push_back("  " + paint(view.hints, {sgr::kDim}));
    return joinLines(lines);
}

Tui::Tui(KeyReader readKey, std::ostream& out, std::mt19937 rng, Options options)
    : readKey_(std::move(readKey)), out_(out), rng_(rng), options_(options) {}

void Tui::run() {
    Next next = Next::Menu;
    while (next != Next::Quit) {
        next = next == Next::Menu ? menu() : playRound();
    }
}

void Tui::draw(const std::string& frame) {
    // Home the cursor and overwrite in place (clearing each line's tail) to
    // avoid flicker from full-screen clears.
    std::string buffer = "\x1b[H";
    std::string_view rest = frame;
    while (!rest.empty()) {
        const auto newline = rest.find('\n');
        buffer += rest.substr(0, newline);
        buffer += "\x1b[K\n";
        rest = newline == std::string_view::npos ? std::string_view{} : rest.substr(newline + 1);
    }
    buffer += "\x1b[J";
    out_ << buffer << std::flush;
}

Tui::Next Tui::menu() {
    int field = 0;
    while (true) {
        draw(renderMenu({.config = config_, .selectedField = field, .score = score_},
                        options_.color));
        const Input input = toInput(readKey_());
        switch (input.action) {
            case Action::Up:
            case Action::Down: field = 1 - field; break;
            case Action::Left:
            case Action::Right: {
                const int step = input.action == Action::Right ? 1 : 2;
                if (field == 0) {
                    const int d = (static_cast<int>(config_.difficulty) + step) % 3;
                    config_.difficulty = static_cast<Difficulty>(d);
                } else {
                    config_.human = opponent(config_.human);
                }
                break;
            }
            case Action::Digit:
                if (field == 0 && input.digit <= 3) {
                    config_.difficulty = static_cast<Difficulty>(input.digit - 1);
                } else if (field == 1 && input.digit <= 2) {
                    config_.human = input.digit == 1 ? Mark::X : Mark::O;
                }
                break;
            case Action::Confirm: return Next::Play;
            case Action::Back:
            case Action::Quit: return Next::Quit;
            case Action::None: break;
        }
    }
}

Tui::Next Tui::playRound() {
    constexpr std::string_view kPlayHints =
        "←↑↓→ move · Enter place · 1-9 quick place · Esc menu · q quit";
    constexpr std::string_view kOverHints = "Enter play again · Esc menu · q quit";

    GameView view;
    view.config = config_;
    view.score = score_;
    view.hints = std::string(kPlayHints);
    const Mark computer = opponent(config_.human);
    std::string notice;

    while (view.board.outcome() == Outcome::InProgress) {
        if (view.board.nextToMove() == computer) {
            view.status = "Computer is thinking…";
            view.tone = Tone::Normal;
            draw(renderGame(view, options_.color));
            if (options_.computerDelay.count() > 0) {
                std::this_thread::sleep_for(options_.computerDelay);
            }
            const auto cell = chooseMove(view.board, computer, config_.difficulty, rng_);
            view.board.place(cell, computer);
            view.lastComputerMove = cell;
            continue;
        }

        if (!notice.empty()) {
            view.status = std::exchange(notice, {});
            view.tone = Tone::Warning;
        } else if (view.lastComputerMove) {
            view.status =
                "Computer played " + std::to_string(*view.lastComputerMove + 1) + ". Your turn.";
            view.tone = Tone::Normal;
        } else {
            view.status = "Your turn.";
            view.tone = Tone::Normal;
        }
        draw(renderGame(view, options_.color));

        const Input input = toInput(readKey_());
        std::optional<std::size_t> target;
        switch (input.action) {
            case Action::Up:
            case Action::Down:
            case Action::Left:
            case Action::Right: view.cursor = moveCursor(view.cursor, input.action); break;
            case Action::Digit:
                view.cursor = static_cast<std::size_t>(input.digit - 1);
                target = view.cursor;
                break;
            case Action::Confirm: target = view.cursor; break;
            case Action::Back: return Next::Menu;
            case Action::Quit: return Next::Quit;
            case Action::None: break;
        }
        if (target) {
            if (view.board.isEmpty(*target)) {
                view.board.place(*target, config_.human);
            } else {
                notice = "Cell " + std::to_string(*target + 1) + " is taken.";
            }
        }
    }

    const auto winner = view.board.winner();
    if (!winner) {
        ++score_.draws;
        view.status = "Draw.";
        view.tone = Tone::Warning;
    } else if (*winner == config_.human) {
        ++score_.wins;
        view.status = "You win!";
        view.tone = Tone::Good;
    } else {
        ++score_.losses;
        view.status = "Computer wins.";
        view.tone = Tone::Bad;
    }
    view.score = score_;
    view.showCursor = false;
    view.hints = std::string(kOverHints);

    while (true) {
        draw(renderGame(view, options_.color));
        switch (toInput(readKey_()).action) {
            case Action::Confirm: return Next::Play;
            case Action::Back: return Next::Menu;
            case Action::Quit: return Next::Quit;
            default: break;
        }
    }
}

}  // namespace ttt
