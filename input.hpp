#pragma once

#include "tui.hpp"
#include <fstream>
#include <iostream>
#include <ostream>

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
std::string ctrl_to_str(const unsigned& key) { return tui::concat("Ctrl", static_cast<char>(key + 64)); }

// Define Control Enum Member
#define _DCEM(X) Ctrl##X
// Define Enum Member
#define DEM(X1, X2, X3, X4) _DCEM(X1), _DCEM(X2), _DCEM(X3), _DCEM(X4)

enum Special {
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
std::ostream& operator<<(std::ostream& os, const Special& special) {
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
std::ostream& operator<<(std::ostream& os, const Arrow& arrow) {
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
    Special special = Special::None;

    bool operator==(const Input& other) const {
        return (this->ch == other.ch && this->is_ch == other.is_ch && this->arrow == other.arrow &&
                this->is_arrow == other.is_arrow && this->special == other.special &&
                this->is_special == other.is_special);
    }
    bool operator==(const char& other) const { return (this->is_ch && this->ch == other); }
    bool operator==(const Special& other) const { return (this->is_special && this->special == other); }
    bool operator==(const Arrow& other) const { return (this->is_arrow && this->arrow == other); }

    bool operator!=(const char& other) const { return !(*this == other); }
    bool operator!=(const Special& other) const { return !(*this == other); }
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
    static Input from_special(const Special& special) {
        Input tmp;
        tmp.special = special;
        tmp.is_special = true;

        return tmp;
    }
    void read(const char& ch) { *this = Input::read_from(ch); }
    static Input read_from(const char& ch) {
        auto pee = 0;
        if /* mostly weird stuff, like: 'Ű' */ (ch < 0) {
            std::cin.ignore();
            return Input::from_special(Special::None);
        }

        if (ch >= 32 && ch <= 126) { // <char>
            return Input::from_char(ch);
        } else if (ch >= 1 && ch <= 26) { // Ctrl<char>
            return Input::from_special(static_cast<Special>(ch));
        }
        switch (ch) {
        case Special::Backspace:
            return Input::from_special(static_cast<Special>(ch));
        case Special::Esc: {
            pee = std::cin.peek();
            // std::cin.clear();
            // std::cerr << "\r\n--well, (): '" << std::cin.sync() << "'--\r\n\n";
            // std::cerr << "\r\n--well, peek: '" << pee << "'--\r\n\n";
            switch (pee) {
            case 91: {
                std::cin.ignore();
                auto sus = std::cin.get();
                switch (sus) {
                case Arrow::Up:
                case Arrow::Down:
                case Arrow::Right:
                case Arrow::Left:
                    return Input::from_arrow(static_cast<Arrow>(sus));
                default:
                    std::cin.ignore(3);
                    break;
                }
                break;
            }
            case 79: {
                std::cin.ignore();
                auto sus = std::cin.get();
                switch (sus) {
                case Special::F1:
                case Special::F2:
                case Special::F3:
                case Special::F4:
                    return Input::from_special(static_cast<Special>(sus));
                default:
                    std::cin.ignore(3);
                    break;
                }
                break;
            }
            default:
                return Input::from_special(Special::Esc);
            }
        default:
            break;
        }
        }
        std::ofstream logf("tui_input_hpp.log", std::ios::app);
        logf << "\nerror: an unknown character:\n\t- code: '" << static_cast<int>(ch) << "'\n\t- display: '" << ch
             << "'\n\n";
        logf.close();
        return Input::from_special(Special::None);
    }
};

std::ostream& operator<<(std::ostream& os, const Input& inp) {
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
