// Self-contained test runner (no third-party dependencies).

#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "tictactoe/ai.hpp"
#include "tictactoe/board.hpp"
#include "tictactoe/game.hpp"

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
    };

    for (const auto& [name, fn] : tests) {
        const int before = g_failures;
        fn();
        std::cout << (g_failures == before ? "[PASS] " : "[FAIL] ") << name << '\n';
    }
    std::cout << (g_failures == 0 ? "All tests passed.\n" : "Some tests failed.\n");
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
