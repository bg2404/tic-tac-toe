# Legacy C++ version

The project's original command-line game, plus a modern rewrite of it. Independent of the web
app: nothing in this directory is part of the Expo/Vercel build.

- [`TICTACTOE.CPP`](TICTACTOE.CPP) — the original single-file Turbo C++ / BGI program for
  16-bit DOS. Kept unchanged for reference; builds only with Turbo C++ (or in DOSBox).
- `src/`, `tests/`, `CMakeLists.txt` — a portable C++20 rewrite for macOS, Linux and Windows
  terminals, described below.

## Features

- Three difficulty levels:
  - **Easy** – plays a random empty cell.
  - **Medium** – takes a winning move if one exists, otherwise blocks yours, otherwise plays randomly.
  - **Hard** – perfect play via minimax; cannot be beaten (best result is a draw).
- Play as Player 1 (X, moves first) or Player 2 (O, moves second).
- Full-screen keyboard interface: coloured marks, highlighted cursor and winning line,
  last computer move underlined, running score across games.
- Plain line-based mode for pipes and scripts (`--plain`).

## Controls

| Key | Menu | Game | Game over |
|---|---|---|---|
| `↑` `↓` / `w` `s` / `k` `j` | Select field | Move cursor | – |
| `←` `→` / `a` `d` / `h` `l` | Change value | Move cursor | – |
| `1`–`9` | Set value | Place mark on that cell | – |
| `Enter` / `Space` | Start | Place mark at cursor | Play again |
| `Esc` / `m` | Quit | Back to menu | Back to menu |
| `q` / `Ctrl-C` | Quit | Quit | Quit |

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
Run the commands below from this `legacy/turbo-cpp/` directory.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

```bash
cmake --build build
```

```bash
./build/tictactoe
```

Options:

| Flag | Effect |
|---|---|
| `--plain` | Line-based prompts instead of the full-screen interface (automatic when stdin/stdout is not a terminal; type `q` to quit) |
| `--no-color` | Disable colours (also disabled when `NO_COLOR` is set to a non-empty value) |
| `--help` | Show usage |

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
src/tictactoe/game.*      plain line-based session
src/tictactoe/tui.*       full-screen interface (rendering, input handling)
src/tictactoe/terminal.*  raw keyboard input and screen control (POSIX / Windows)
tests/tests.cpp           unit and integration tests
```

## Original README

The Turbo C++ version's README, as written:

> # tic-tac-toe
>
> The world famous game made in command line avatar for 16-bit system in Turbo C++.
> This progrm was written using TURBO C++ on a Windows 7 PC/
>
> This Offers three difficulty versions - Easy (Computer just plays anywhere without thinking)
>                 			Medium (Computer knows basic rules but still lack any tricks)
> 					Difficult (Conputer is unbeatable knows all the tricks)
>
> Each Difficulty level offers two variants in which Human can play as Player 1 or Player 2
