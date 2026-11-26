#include <exception>
#include <iostream>
#include <random>
#include <string_view>

#include "tictactoe/game.hpp"
#include "tictactoe/terminal.hpp"
#include "tictactoe/tui.hpp"

namespace {

constexpr std::string_view kUsage =
    "Usage: tictactoe [--plain] [--no-color] [--help]\n"
    "\n"
    "  --plain     Line-based prompts instead of the full-screen interface\n"
    "              (used automatically when stdin/stdout is not a terminal).\n"
    "  --no-color  Disable colours (also honours the NO_COLOR environment variable).\n"
    "  --help      Show this help.\n";

}  // namespace

int main(int argc, char* argv[]) {
    bool plain = false;
    bool color = !ttt::term::noColorRequested();
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--plain") {
            plain = true;
        } else if (arg == "--no-color") {
            color = false;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << kUsage;
            return 0;
        } else {
            std::cerr << "unknown option: " << arg << "\n\n" << kUsage;
            return 2;
        }
    }

    try {
        std::mt19937 rng{std::random_device{}()};
        if (plain || !ttt::term::isInteractive()) {
            ttt::Session(std::cin, std::cout, rng).run();
            return 0;
        }
        const ttt::term::RawMode raw;
        ttt::Tui::Options options;
        options.color = color;
        ttt::Tui(ttt::term::readKey, std::cout, rng, options).run();
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
