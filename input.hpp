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
inline void set_non_blocking(bool enable) {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, enable ? flags | O_NONBLOCK : flags & ~O_NONBLOCK);
}
#endif

// sorry for this ugly code, I really feel very bad about it.
// damn macros, because given `MyEnum::Core` you can't do
// `cout << MyEnum::Core; assert(cout.string() == "Core"|"MyEnum::Core")`

// Define Control Enum Member
#define _DCEM(X) Ctrl##X
// Define Enum Member
#define DEM(X1, X2, X3, X4) _DCEM(X1), _DCEM(X2), _DCEM(X3), _DCEM(X4)

// TODO: maybe add keys: F(5-12) NOTE: shall need an own `struct F {num: u8}`, ...
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
    Insert = 50,
    Delete = 51,
    PageUp = 53,
    PageDown = 54,
    End = 70,
    Home = 72,
    F1 = 80,
    F2 = 81,
    F3 = 82,
    F4 = 83,
    ShiftTab = 90,
    Backspace = 127,
    None,
};
#undef DCEM
#undef DEM

enum Arrow {
    Up = 65,
    Down = 66,
    Right = 67,
    Left = 68,
};

// `case`, return as `string`
#define CRS(X)                                                                                                         \
    case X:                                                                                                            \
        return os << #X;
// 4 case, return string-s
#define CRSS(A, B, C, D) CRS(A) CRS(B) CRS(C) CRS(D)

// convert `Ctrl+<character>` sequence to a `string` as eg.: `CtrlX`
inline std::string ctrl_to_str(const unsigned& key) { return tui::concat("Ctrl", static_cast<char>(key + 64)); }

inline std::ostream& operator<<(std::ostream& os, const SpecKey& special) {
    switch (special) {
        CRSS(Esc, Tab, Backspace, Enter);
        CRSS(F1, F2, F3, F4);
        CRSS(Home, End, PageUp, PageDown);
        CRSS(ShiftTab, Insert, Delete, None);
    default:
        if (special >= 1 && special <= 26) {
            return os << ctrl_to_str(special);
        }
        os << "Unknown";
    }
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const Arrow& arrow) {
    switch (arrow) {
        CRSS(Up, Down, Right, Left);
    default:
        os << "Unknown";
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
    static Input read_helper(reader_fn get_char) {
        char byte = get_char();

        char ignore_byte = 0;
        auto input = Input::from_special(SpecKey::None);
#ifdef _WIN32
        if (byte == 0 || byte == 224) {
            char next_byte = get_char();
            switch (next_byte) {
            case 59:
                return Input::from_special(SpecKey::F1);
            case 60:
                return Input::from_special(SpecKey::F2);
            case 61:
                return Input::from_special(SpecKey::F3);
            case 62:
                return Input::from_special(SpecKey::F4);
            default:
                break;
            }
        } else if (byte == -32) {
            char next_byte = get_char();
            switch (next_byte) {
            case 72:
                return Input::from_arrow(Arrow::Up);
            case 75:
                return Input::from_arrow(Arrow::Left);
            case 77:
                return Input::from_arrow(Arrow::Right);
            case 80:
                return Input::from_arrow(Arrow::Down);
            case 83:
                return Input::from_special(SpecKey::Delete);
            case 81:
                return Input::from_special(SpecKey::PageDown);
            case 71:
                return Input::from_special(SpecKey::Home);
            case 73:
                return Input::from_special(SpecKey::PageUp);
            case 79:
                return Input::from_special(SpecKey::End);
            case 82:
                return Input::from_special(SpecKey::Insert);
            default:
                break;
            }
        }
#else
        if (byte < 0) {
            ignore_byte = get_char();
        }
#endif
        if (byte >= 32 && byte <= 126) { // <char>
            input = Input::from_char(byte);
        } else if (byte >= 1 && byte <= 26) { // Ctrl<char>
            input = Input::from_special(static_cast<SpecKey>(byte));
        }

        switch (byte) {
        case SpecKey::Backspace:
            input = Input::from_special(static_cast<SpecKey>(byte));
            break;
        case SpecKey::Esc: {
#ifndef _WIN32
            set_non_blocking(false); // Temporarily make stdin non-blocking
            char next_byte = get_char();

            switch (next_byte) {
            case 91: {
                char special = get_char();
                switch (special) {
                case Arrow::Up:
                case Arrow::Down:
                case Arrow::Right:
                case Arrow::Left:
                    input = Input::from_arrow(static_cast<Arrow>(special));
                    break;
                case SpecKey::End:
                case SpecKey::Home:
                case SpecKey::ShiftTab:
                    input = Input::from_special(static_cast<SpecKey>(special));
                    break;
                case SpecKey::Insert:
                case SpecKey::Delete:
                case SpecKey::PageUp:
                case SpecKey::PageDown:
                    ignore_byte = get_char(); // ~
                    input = Input::from_special(static_cast<SpecKey>(special));
                    break;
                default:
                    // ignore_byte = _getch(3);
                    break;
                }
                break;
            }
            case 79: {
                // get function key character
                char f_key = get_char();
                switch (f_key) {
                case SpecKey::F1:
                case SpecKey::F2:
                case SpecKey::F3:
                case SpecKey::F4:
                    input = Input::from_special(static_cast<SpecKey>(f_key));
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                input = Input::from_special(SpecKey::Esc);
            }
            set_non_blocking(false); // Temporarily make stdin non-blocking
#else
            input = Input::from_special(SpecKey::Esc);
#endif
        }
        default:
            break;
        }
        return input;
    }

  public:
// TODO: test
#ifdef _WIN32 // windows
    static char read_ch() {
        char tmp = _getch();
        return tmp;
    }
    static Input read() {
        // read raw input
        return Input::read_helper(Input::read_ch);
    }
#else // not windows
    static char read_ch() {
        char tmp = 0;
        ::read(STDIN_FILENO, &tmp, 1);
        return tmp;
    }
    static Input read() {
        // read raw input
        return Input::read_helper(Input::read_ch);
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
