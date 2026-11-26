#include <exception>
#include <iostream>
#include <random>

#include "tictactoe/game.hpp"

int main() {
    try {
        ttt::Session session(std::cin, std::cout, std::mt19937{std::random_device{}()});
        session.run();
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
