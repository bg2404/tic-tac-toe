#include "tictactoe/terminal.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <io.h>
#include <windows.h>
#else
#include <poll.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#endif

namespace ttt::term {
namespace {

constexpr std::string_view kEnterScreen = "\x1b[?1049h\x1b[?25l";  // alt screen, hide cursor
constexpr std::string_view kLeaveScreen = "\x1b[0m\x1b[?25h\x1b[?1049l";

Key charKey(char c) noexcept {
    switch (c) {
        case '\r':
        case '\n': return {KeyType::Enter};
        case ' ': return {KeyType::Space};
        case '\x1b': return {KeyType::Escape};
        case '\x7f':
        case '\b': return {KeyType::Backspace};
        case '\x03': return {KeyType::Interrupt};
        case '\x04': return {KeyType::Eof};
        default: break;
    }
    if (static_cast<unsigned char>(c) >= 0x20 && static_cast<unsigned char>(c) < 0x7f) {
        return {KeyType::Char, c};
    }
    return {};
}

#ifdef _WIN32

DWORD g_savedOutMode = 0;
UINT g_savedOutCp = 0;
bool g_haveOutMode = false;

#else

termios g_savedTermios{};

#endif

}  // namespace

Key parseKey(std::string_view bytes) noexcept {
    if (bytes.empty()) {
        return {};
    }
    if (bytes.size() == 1) {
        return charKey(bytes[0]);
    }
    // CSI ("\x1b[") or SS3 ("\x1bO") arrow sequences.
    if (bytes.size() == 3 && bytes[0] == '\x1b' && (bytes[1] == '[' || bytes[1] == 'O')) {
        switch (bytes[2]) {
            case 'A': return {KeyType::Up};
            case 'B': return {KeyType::Down};
            case 'C': return {KeyType::Right};
            case 'D': return {KeyType::Left};
            default: break;
        }
    }
    return {};
}

bool noColorRequested() {
#ifdef _MSC_VER
    char* value = nullptr;
    std::size_t length = 0;
    if (_dupenv_s(&value, &length, "NO_COLOR") != 0 || value == nullptr) {
        return false;
    }
    const bool set = value[0] != '\0';
    std::free(value);
    return set;
#else
    const char* value = std::getenv("NO_COLOR");
    return value != nullptr && value[0] != '\0';
#endif
}

#ifdef _WIN32

bool isInteractive() noexcept { return _isatty(_fileno(stdin)) && _isatty(_fileno(stdout)); }

Key readKey() {
    const int c = _getch();
    if (c == 0 || c == 0xE0) {  // Extended key prefix.
        switch (_getch()) {
            case 72: return {KeyType::Up};
            case 80: return {KeyType::Down};
            case 75: return {KeyType::Left};
            case 77: return {KeyType::Right};
            default: return {};
        }
    }
    if (c == 0x1A) {  // Ctrl-Z
        return {KeyType::Eof};
    }
    return charKey(static_cast<char>(c));
}

RawMode::RawMode() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleMode(out, &g_savedOutMode)) {
        g_haveOutMode = true;
        SetConsoleMode(
            out, g_savedOutMode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    g_savedOutCp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
    std::cout << kEnterScreen << std::flush;
}

RawMode::~RawMode() {
    std::cout << kLeaveScreen << std::flush;
    if (g_haveOutMode) {
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), g_savedOutMode);
    }
    SetConsoleOutputCP(g_savedOutCp);
}

#else

bool isInteractive() noexcept { return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO); }

Key readKey() {
    std::string bytes(1, '\0');
    while (true) {
        const ssize_t n = read(STDIN_FILENO, bytes.data(), 1);
        if (n == 1) {
            break;
        }
        if (n == 0 || errno != EINTR) {
            return {KeyType::Eof};
        }
    }

    // An escape sequence arrives in one burst; a lone ESC does not.
    if (bytes[0] == '\x1b') {
        pollfd pfd{STDIN_FILENO, POLLIN, 0};
        constexpr int kEscapeTimeoutMs = 30;
        while (bytes.size() < 8 && poll(&pfd, 1, kEscapeTimeoutMs) > 0) {
            char c = '\0';
            if (read(STDIN_FILENO, &c, 1) != 1) {
                break;
            }
            bytes.push_back(c);
            if (bytes.size() >= 3 && c >= '@' && c <= '~') {
                break;  // Final byte of a CSI/SS3 sequence.
            }
        }
    }
    return parseKey(bytes);
}

RawMode::RawMode() {
    if (tcgetattr(STDIN_FILENO, &g_savedTermios) != 0) {
        throw std::runtime_error("cannot read terminal attributes");
    }
    termios raw = g_savedTermios;
    raw.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO | ISIG | IEXTEN);
    raw.c_iflag &= ~static_cast<tcflag_t>(IXON | ICRNL);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
        throw std::runtime_error("cannot set terminal attributes");
    }
    std::cout << kEnterScreen << std::flush;
}

RawMode::~RawMode() {
    std::cout << kLeaveScreen << std::flush;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_savedTermios);
}

#endif

}  // namespace ttt::term
