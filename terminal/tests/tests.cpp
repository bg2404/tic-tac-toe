// Self-contained test runner (no third-party dependencies).

#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "tictactoe/ai.hpp"
#include "tictactoe/board.hpp"
#include "tictactoe/game.hpp"
#include "tictactoe/terminal.hpp"
#include "tictactoe/tui.hpp"

namespace {

int g_failures = 0;

void check(bool condition, std::string_view expr, std::string_view file, int line) {
    if (!condition) {
        ++g_failures;
        std::cerr << file << ':' << line << ": CHECK failed: " << expr << '\n';
    }
}

#define CHECK(expr) check(static_cast<bool>(expr), #expr, __FILE__, __LINE__)

using ttt::Board;
using ttt::Difficulty;
using ttt::Mark;
using ttt::Outcome;

Board boardFrom(std::string_view layout) {
    // layout: 9 chars of 'X', 'O' or '.', row-major.
    Board b;
    for (std::size_t i = 0; i < layout.size(); ++i) {
        if (layout[i] == 'X') b.place(i, Mark::X);
        if (layout[i] == 'O') b.place(i, Mark::O);
    }
    return b;
}

void testPlaceRejectsInvalid() {
    Board b;
    CHECK(b.place(0, Mark::X));
    CHECK(!b.place(0, Mark::O));
    CHECK(!b.place(9, Mark::O));
    CHECK(!b.place(1, Mark::Empty));
    CHECK(b.at(0) == Mark::X);
}

void testWinnerDetection() {
    for (const auto& line : Board::kLines) {
        Board b;
        for (const auto cell : line) b.place(cell, Mark::O);
        CHECK(b.winner() == Mark::O);
        CHECK(b.outcome() == Outcome::OWins);
    }
    CHECK(!boardFrom("XOXXOOOXX").winner().has_value());
    CHECK(boardFrom("XOXXOOOXX").outcome() == Outcome::Draw);
    CHECK(Board{}.outcome() == Outcome::InProgress);
}

void testNextToMove() {
    CHECK(Board{}.nextToMove() == Mark::X);
    CHECK(boardFrom("X........").nextToMove() == Mark::O);
    CHECK(boardFrom("XO.......").nextToMove() == Mark::X);
}

void testWinningMoves() {
    const Board b = boardFrom("XX.OO....");
    CHECK(ttt::winningMoves(b, Mark::X) == std::vector<std::size_t>{2});
    CHECK(ttt::winningMoves(b, Mark::O) == std::vector<std::size_t>{5});
}

void testMediumWinsThenBlocks() {
    std::mt19937 rng(1);
    // X can win at 2; must prefer winning over blocking O at 5.
    CHECK(ttt::chooseMove(boardFrom("XX.OO...."), Mark::X, Difficulty::Medium, rng) == 2);
    // O cannot win; must block X at 2.
    CHECK(ttt::chooseMove(boardFrom("XX..O...."), Mark::O, Difficulty::Medium, rng) == 2);
}

void testEasyPlaysLegalMoves() {
    std::mt19937 rng(7);
    const Board b = boardFrom("XOXOX....");
    for (int i = 0; i < 100; ++i) {
        CHECK(b.isEmpty(ttt::chooseMove(b, Mark::O, Difficulty::Easy, rng)));
    }
}

void testHardTakesImmediateWin() {
    // O to move: winning at 8 beats any other move.
    CHECK(ttt::bestMoves(boardFrom("OX.XO.X.."), Mark::O) == std::vector<std::size_t>{8});
}

// Explores every opponent reply and every optimal AI choice; the AI must never lose.
void exploreHard(Board& board, Mark ai, int& positions) {
    const Outcome o = board.outcome();
    if (o != Outcome::InProgress) {
        const Outcome loss = ai == Mark::X ? Outcome::OWins : Outcome::XWins;
        CHECK(o != loss);
        return;
    }
    ++positions;
    const Mark toMove = board.nextToMove();
    const auto moves = toMove == ai ? ttt::bestMoves(board, ai) : board.emptyCells();
    CHECK(!moves.empty());
    for (const auto cell : moves) {
        board.place(cell, toMove);
        exploreHard(board, ai, positions);
        board.clear(cell);
    }
}

void testHardNeverLoses() {
    for (const Mark ai : {Mark::X, Mark::O}) {
        Board b;
        int positions = 0;
        exploreHard(b, ai, positions);
        CHECK(positions > 0);
    }
}

void testRender() {
    CHECK(ttt::render(boardFrom("X...O....")) ==
          " X | 2 | 3\n---+---+---\n 4 | O | 6\n---+---+---\n 7 | 8 | 9\n");
}

void testSessionFullGame() {
    // Human is X on Hard; computer must at least draw. Feed every cell in
    // order; occupied cells are rejected and the next line is tried.
    std::istringstream in("3\n1\n1\n2\n3\n4\n5\n6\n7\n8\n9\nn\n");
    std::ostringstream out;
    ttt::Session session(in, out, std::mt19937(42));
    session.run();
    const std::string text = out.str();
    CHECK(text.find("You win!") == std::string::npos);
    CHECK(text.find("Goodbye.") != std::string::npos);
}

void testSessionInvalidInputAndQuit() {
    std::istringstream in("abc\n0\n4\nq\n");
    std::ostringstream out;
    ttt::Session session(in, out, std::mt19937(1));
    session.run();
    const std::string text = out.str();
    CHECK(text.find("Invalid input") != std::string::npos);
    CHECK(text.find("Goodbye.") != std::string::npos);
    CHECK(text.find("Choose your side") == std::string::npos);
}

void testSessionEndOfInput() {
    std::istringstream in("2\n2\n");
    std::ostringstream out;
    ttt::Session session(in, out, std::mt19937(1));
    session.run();
    CHECK(out.str().find("Goodbye.") != std::string::npos);
}

// ---- terminal / TUI ----

using ttt::term::Key;
using ttt::term::KeyType;

std::string stripAnsi(const std::string& s) {
    static const std::regex kEscape("\x1b\\[[0-9;?]*[A-Za-z]");
    return std::regex_replace(s, kEscape, "");
}

// Text of the last frame drawn by the TUI, without escape codes.
std::string lastFrame(const std::string& output) {
    const auto pos = output.rfind("\x1b[H");
    return stripAnsi(pos == std::string::npos ? output : output.substr(pos));
}

ttt::Tui::KeyReader scripted(std::vector<Key> keys) {
    return [keys = std::move(keys), i = std::size_t{0}]() mutable {
        return i < keys.size() ? keys[i++] : Key{KeyType::Eof};
    };
}

Key ch(char c) { return {KeyType::Char, c}; }

void testParseKey() {
    using ttt::term::parseKey;
    CHECK(parseKey("\x1b[A") == Key{KeyType::Up});
    CHECK(parseKey("\x1bOB") == Key{KeyType::Down});
    CHECK(parseKey("\x1b[C") == Key{KeyType::Right});
    CHECK(parseKey("\x1b[D") == Key{KeyType::Left});
    CHECK(parseKey("\x1b") == Key{KeyType::Escape});
    CHECK(parseKey("\r") == Key{KeyType::Enter});
    CHECK(parseKey("\n") == Key{KeyType::Enter});
    CHECK(parseKey(" ") == Key{KeyType::Space});
    CHECK(parseKey("\x03") == Key{KeyType::Interrupt});
    CHECK(parseKey("q") == ch('q'));
    CHECK(parseKey("\x1b[5~") == Key{KeyType::None});
    CHECK(parseKey("") == Key{KeyType::None});
}

void testWinningLine() {
    CHECK((boardFrom("X..OX.O.X").winningLine() == Board::Line{0, 4, 8}));
    CHECK(!boardFrom("XO.......").winningLine().has_value());
}

void testRenderGameLayout() {
    ttt::GameView view;
    view.board = boardFrom("X...O....");
    view.showCursor = false;
    const std::string text = stripAnsi(ttt::renderGame(view, false));
    CHECK(text.find("   X   │   2   │   3   ") != std::string::npos);
    CHECK(text.find("   4   │   O   │   6   ") != std::string::npos);
    CHECK(text.find("───────┼───────┼───────") != std::string::npos);
    CHECK(text.find("Computer  0") != std::string::npos);
}

void testRenderColorToggle() {
    ttt::GameView view;
    view.board = boardFrom("X........");
    CHECK(ttt::renderGame(view, true).find("\x1b[1;96m") != std::string::npos);
    CHECK(ttt::renderGame(view, false).find("96") == std::string::npos);
}

void testRenderMenu() {
    ttt::MenuView view;
    view.config = {.difficulty = Difficulty::Hard, .human = Mark::O};
    const std::string text = stripAnsi(ttt::renderMenu(view, false));
    CHECK(text.find("‹ Hard ›") != std::string::npos);
    CHECK(text.find("O  (moves second)") != std::string::npos);
    CHECK(text.find("Perfect play") != std::string::npos);
}

void testTuiCursorPlacement() {
    // Menu: Left (Medium -> Easy), Enter. Game: Up (5 -> 2), Enter, quit.
    std::ostringstream out;
    ttt::Tui tui(
        scripted({{KeyType::Left}, {KeyType::Enter}, {KeyType::Up}, {KeyType::Enter}, ch('q')}),
        out, std::mt19937(3), {.color = false, .computerDelay = {}});
    tui.run();
    const std::string frame = lastFrame(out.str());
    CHECK(frame.find("Level     Easy") != std::string::npos);
    CHECK(frame.find("│   X   │") != std::string::npos);
    CHECK(frame.find("Computer played") != std::string::npos);
}

void testTuiTakenCellNotice() {
    std::ostringstream out;
    ttt::Tui tui(scripted({{KeyType::Enter}, ch('5'), ch('5')}), out, std::mt19937(1),
                 {.color = false, .computerDelay = {}});
    tui.run();
    CHECK(lastFrame(out.str()).find("Cell 5 is taken.") != std::string::npos);
}

void testTuiFullGameUpdatesScore() {
    // Hard, play as O, then try every cell twice; then back to menu and quit.
    std::vector<Key> keys{{KeyType::Right}, {KeyType::Down}, {KeyType::Right}, {KeyType::Enter}};
    for (int pass = 0; pass < 2; ++pass) {
        for (char c = '1'; c <= '9'; ++c) keys.push_back(ch(c));
    }
    keys.push_back({KeyType::Escape});
    keys.push_back(ch('q'));
    std::ostringstream out;
    ttt::Tui tui(scripted(std::move(keys)), out, std::mt19937(5),
                 {.color = false, .computerDelay = {}});
    tui.run();
    const ttt::Score& s = tui.score();
    CHECK(s.wins == 0);
    CHECK(s.losses + s.draws == 1);
    CHECK(lastFrame(out.str()).find("Score  You 0") != std::string::npos);
}

}  // namespace

int main() {
    const std::vector<std::pair<std::string_view, std::function<void()>>> tests{
        {"place rejects invalid", testPlaceRejectsInvalid},
        {"winner detection", testWinnerDetection},
        {"next to move", testNextToMove},
        {"winning moves", testWinningMoves},
        {"medium wins then blocks", testMediumWinsThenBlocks},
        {"easy plays legal moves", testEasyPlaysLegalMoves},
        {"hard takes immediate win", testHardTakesImmediateWin},
        {"hard never loses", testHardNeverLoses},
        {"render", testRender},
        {"session full game", testSessionFullGame},
        {"session invalid input and quit", testSessionInvalidInputAndQuit},
        {"session end of input", testSessionEndOfInput},
        {"parse key", testParseKey},
        {"winning line", testWinningLine},
        {"render game layout", testRenderGameLayout},
        {"render color toggle", testRenderColorToggle},
        {"render menu", testRenderMenu},
        {"tui cursor placement", testTuiCursorPlacement},
        {"tui taken cell notice", testTuiTakenCellNotice},
        {"tui full game updates score", testTuiFullGameUpdatesScore},
    };

    for (const auto& [name, fn] : tests) {
        const int before = g_failures;
        fn();
        std::cout << (g_failures == before ? "[PASS] " : "[FAIL] ") << name << '\n';
    }
    std::cout << (g_failures == 0 ? "All tests passed.\n" : "Some tests failed.\n");
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
