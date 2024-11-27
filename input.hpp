#pragma once

#include "tui.hpp"
#include <iostream>

// Platform-specific includes
#ifdef _WIN32
#include <conio.h>   // For _kbhit() and _getch() on Windows
#include <windows.h> // For GetConsoleMode and SetConsoleMode
#else
#include <fcntl.h>   // For fcntl() on Unix-like systems
#include <termios.h> // For termios on Unix-like systems
#include <unistd.h>  // For read(), usleep() on Unix-like systems
#endif

// Function to set stdin non-blocking on Unix-like systems
#ifndef _WIN32
void set_non_blocking(bool enable) {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, enable ? flags | O_NONBLOCK : flags & ~O_NONBLOCK);
}
#endif

// sorry for this ugly code, I really feel very bad about it.
// damn macros, because given `MyEnum::Core` you can't do
// `cout << MyEnum::Core; assert(cout.string() == "Core"|"MyEnum::Core")`

// `case`, return as `string`
#define CRS(X)                                                                                                         \
    case X:                                                                                                            \
        return os << #X;
// 4 case, return string-s
#define CRSS(A, B, C, D) CRS(A) CRS(B) CRS(C) CRS(D)

// convert `Ctrl+<character>` sequence to a `string` as eg.: `CtrlX`
inline std::string ctrl_to_str(const unsigned& key) { return tui::concat("Ctrl", static_cast<char>(key + 64)); }

// Define Control Enum Member
#define _DCEM(X) Ctrl##X
// Define Enum Member
#define DEM(X1, X2, X3, X4) _DCEM(X1), _DCEM(X2), _DCEM(X3), _DCEM(X4)

// TODO: maybe add keys: del, F(5-12), home, end, pg(up,down), ShiftTab, ...
enum SpecKey {
    CtrlA = 1,
    DEM(B, C, D, E),
    DEM(F, G, H, I),
    Tab = 9,
    DEM(J, K, L, M),
    Enter = 13,
    DEM(N, O, P, Q),
    DEM(R, S, T, U),
    DEM(V, W, X, Y),
    CtrlZ = 26,
    Esc = 27,
    F1 = 80,
    F2 = 81,
    F3 = 82,
    F4 = 83,
    Backspace = 127,
    None,
};
#undef DCEM
#undef DEM
inline std::ostream& operator<<(std::ostream& os, const SpecKey& special) {
    switch (special) {
        CRSS(Esc, Tab, Backspace, Enter);
        CRSS(F1, F2, F3, F4);

        CRS(None);
    default:
        if (special >= 1 && special <= 26) {
            return os << ctrl_to_str(special);
        }
        os << "Unknown";
    }
    return os;
}
enum Arrow {
    Up = 65,
    Down = 66,
    Right = 67,
    Left = 68,
};
inline std::ostream& operator<<(std::ostream& os, const Arrow& arrow) {
    switch (arrow) {
        CRSS(Up, Down, Right, Left);
    default:
        os << "Unknown";
        break;
    }
    return os;
}
#undef CRS
#undef CRSS

struct Input {
    bool is_ch = false;
    bool is_arrow = false;
    bool is_special = false;

    char ch = '\0';
    Arrow arrow = static_cast<Arrow>(0);
    SpecKey special = SpecKey::None;

    bool operator==(const Input& other) const {
        return (this->ch == other.ch && this->is_ch == other.is_ch && this->arrow == other.arrow &&
                this->is_arrow == other.is_arrow && this->special == other.special &&
                this->is_special == other.is_special);
    }
    bool operator==(const char& other) const { return (this->is_ch && this->ch == other); }
    bool operator==(const SpecKey& other) const { return (this->is_special && this->special == other); }
    bool operator==(const Arrow& other) const { return (this->is_arrow && this->arrow == other); }

    bool operator!=(const char& other) const { return !(*this == other); }
    bool operator!=(const SpecKey& other) const { return !(*this == other); }
    bool operator!=(const Arrow& other) const { return !(*this == other); }
    bool operator!=(const Input& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Input& inp);

    static Input from_arrow(const Arrow& arrow) {
        Input tmp;
        tmp.arrow = arrow;
        tmp.is_arrow = true;

        return tmp;
    }
    static Input from_char(const char& ch) {
        Input tmp;
        tmp.ch = ch;
        tmp.is_ch = true;

        return tmp;
    }
    static Input from_special(const SpecKey& special) {
        Input tmp;
        tmp.special = special;
        tmp.is_special = true;

        return tmp;
    }

  private:
    using reader_fn = char (*)();
    using esc_setup_fn = void (*)(bool);
    static Input read_helper(reader_fn get_char, esc_setup_fn dont_block) {
        char byte = get_char();

        Input input;
        if (byte < 0) {
            char ignore_byte;
            ignore_byte = get_char();
            input = Input::from_special(SpecKey::None);
        } else if (byte >= 32 && byte <= 126) { // <char>
            input = Input::from_char(byte);
        } else if (byte >= 1 && byte <= 26) { // Ctrl<char>
            input = Input::from_special(static_cast<SpecKey>(byte));
        }

        switch (byte) {
        case SpecKey::Backspace:
            input = Input::from_special(static_cast<SpecKey>(byte));
            break;
        case SpecKey::Esc: {
            dont_block(true);
            char next_byte;
            next_byte = get_char();
            // char ignore;

            switch (next_byte) {
            case 91: {
                char arrow;
                // get arrow key character
                arrow = get_char();
                switch (arrow) {
                case Arrow::Up:
                case Arrow::Down:
                case Arrow::Right:
                case Arrow::Left:
                    input = Input::from_arrow(static_cast<Arrow>(arrow));
                    break;
                default:
                    // ignore = _getch(3);
                    break;
                }
                break;
            }
            case 79: {
                char f_key;
                // get function key character
                f_key = get_char();
                switch (f_key) {
                case SpecKey::F1:
                case SpecKey::F2:
                case SpecKey::F3:
                case SpecKey::F4:
                    input = Input::from_special(static_cast<SpecKey>(f_key));
                    break;
                default:
                    // ignore = _getch(3);
                    break;
                }
                break;
            }
            default:
                input = Input::from_special(SpecKey::Esc);
                break;
            }
            dont_block(false);
            break;
        }
        }
        if (input == Input()) {
            input = Input::from_special(SpecKey::None);
        }
        return input;
    }

  public:
// TODO: test
#ifdef _WIN32 // windows
    static void noop(bool on) {}
    static char read_ch() {
        char tmp;
        tmp = _getch();
        return tmp;
    }
    static Input read() {
        char byte;

        // read raw input
        if (_kbhit()) {
            return Input::read_helper(_getch, Input::noop);
        }
        return Input::from_special(SpecKey::None);
    }
#else // not windows
    static void non_blocking(bool on) {
        set_non_blocking(on); // Temporarily make stdin non-blocking
    }
    static char read_ch() {
        char tmp;
        ::read(STDIN_FILENO, &tmp, 1);
        return tmp;
    }
    static Input read() {
        char byte;

        // read raw input
        // if (::read(STDIN_FILENO, &byte, 1) == 1) {
        return Input::read_helper(Input::read_ch, Input::non_blocking);
        // }
        // return Input::from_special(SpecKey::None);
    }
#endif
};

inline std::ostream& operator<<(std::ostream& os, const Input& inp) {
    if (inp.is_ch) {
        os << "character: '" << inp.ch << "'";
    } else if (inp.is_arrow) {
        os << "arrow: " << inp.arrow;
    } else if (inp.is_special) {
        os << "special: " << inp.special;
    } else if (inp == Input()) {
        os << "unset";
    } else {
        os << "error: unknown input";
    }
    return os;
}
