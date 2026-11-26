# Terminal version (C++20)

Console tic-tac-toe against the computer, written in portable C++20. A modern rewrite of the
original Turbo C++ game in [`../legacy/turbo-cpp/`](../legacy/turbo-cpp/). Independent of the web
app: nothing here is part of the Expo/Vercel build.

## Features

- Three difficulty levels:
  - **Easy** – plays a random empty cell.
  - **Medium** – takes a winning move if one exists, otherwise blocks yours, otherwise plays randomly.
  - **Hard** – perfect play via minimax; cannot be beaten (best result is a draw).
- Play as Player 1 (X, moves first) or Player 2 (O, moves second).
- Input validation; type `q` at any prompt to quit.

Cells are numbered 1–9, left to right, top to bottom:

```
 1 | 2 | 3
---+---+---
 4 | 5 | 6
---+---+---
 7 | 8 | 9
```

## Build

Requirements: CMake ≥ 3.20 and a C++20 compiler (GCC ≥ 11, Clang ≥ 14, MSVC 2022).
Run the commands below from this `terminal/` directory.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build
```

```bash
./build/tictactoe
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

The test suite includes an exhaustive search over every opponent move sequence, verifying that the Hard level never loses as either X or O.

CMake options:

| Option | Default | Effect |
|---|---|---|
| `TICTACTOE_BUILD_TESTS` | `ON` | Build the `tictactoe_tests` target |
| `TICTACTOE_WARNINGS_AS_ERRORS` | `OFF` | Add `-Werror` / `/WX` |

## Layout

```
src/main.cpp              entry point
src/tictactoe/board.*     board state, win/draw detection
src/tictactoe/ai.*        computer strategies per difficulty
src/tictactoe/game.*      console session (menus, game loop, rendering)
tests/tests.cpp           unit and integration tests
```

## History

Rewrite of the single-file Turbo C++ / BGI program for 16-bit DOS kept in
[`../legacy/turbo-cpp/TICTACTOE.CPP`](../legacy/turbo-cpp/TICTACTOE.CPP).
