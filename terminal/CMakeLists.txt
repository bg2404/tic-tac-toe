cmake_minimum_required(VERSION 3.20)

project(tictactoe
    VERSION 2.0.0
    DESCRIPTION "Console tic-tac-toe with three computer difficulty levels"
    LANGUAGES CXX)

option(TICTACTOE_BUILD_TESTS "Build unit tests" ON)
option(TICTACTOE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Shared compile settings.
add_library(tictactoe_options INTERFACE)
target_compile_features(tictactoe_options INTERFACE cxx_std_20)
if(MSVC)
    target_compile_options(tictactoe_options INTERFACE /W4 /permissive-
        $<$<BOOL:${TICTACTOE_WARNINGS_AS_ERRORS}>:/WX>)
else()
    target_compile_options(tictactoe_options INTERFACE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow
        $<$<BOOL:${TICTACTOE_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

add_library(tictactoe_core
    src/tictactoe/board.cpp
    src/tictactoe/ai.cpp
    src/tictactoe/game.cpp)
target_include_directories(tictactoe_core PUBLIC src)
target_link_libraries(tictactoe_core PUBLIC tictactoe_options)

add_executable(tictactoe src/main.cpp)
target_link_libraries(tictactoe PRIVATE tictactoe_core)

install(TARGETS tictactoe RUNTIME DESTINATION bin)

if(TICTACTOE_BUILD_TESTS)
    enable_testing()
    add_executable(tictactoe_tests tests/tests.cpp)
    target_link_libraries(tictactoe_tests PRIVATE tictactoe_core)
    add_test(NAME tictactoe_tests COMMAND tictactoe_tests)
endif()
