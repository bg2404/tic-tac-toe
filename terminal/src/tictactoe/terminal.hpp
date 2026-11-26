#pragma once

#include <cstdint>
#include <string_view>

namespace ttt::term {

enum class KeyType : std::uint8_t {
    None,  // Unrecognised input; ignore.
    Up,
    Down,
    Left,
    Right,
    Enter,
    Space,
    Escape,
    Backspace,
    Interrupt,  // Ctrl-C
    Eof,        // Ctrl-D / end of input
    Char,       // Printable character in `ch`.
};

struct Key {
    KeyType type = KeyType::None;
    char ch = '\0';

    friend bool operator==(const Key&, const Key&) = default;
};

// Decodes one complete key sequence as read from a POSIX terminal in raw mode
// (e.g. "\x1b[A" for the up arrow).
[[nodiscard]] Key parseKey(std::string_view bytes) noexcept;

// True when both stdin and stdout are attached to a terminal.
[[nodiscard]] bool isInteractive() noexcept;

// True when the NO_COLOR environment variable is set to a non-empty value.
[[nodiscard]] bool noColorRequested();

// Blocking read of one key from stdin. Requires an active RawMode.
[[nodiscard]] Key readKey();

// RAII: switches the terminal to unbuffered, no-echo input, enables ANSI
// output (Windows), enters the alternate screen and hides the cursor.
// Everything is restored on destruction.
class RawMode {
public:
    RawMode();
    ~RawMode();
    RawMode(const RawMode&) = delete;
    RawMode& operator=(const RawMode&) = delete;
    RawMode(RawMode&&) = delete;
    RawMode& operator=(RawMode&&) = delete;
};

}  // namespace ttt::term
