// tui.hpp
#pragma once

#ifdef _WIN32 // windows

#include <wchar.h>
#include <windows.h>

#else // not windows

#include <csignal>
#include <err.h>       /* err */
#include <fcntl.h>     /* open */
#include <sys/ioctl.h> /* ioctl, TIOCGWINSZ */
#include <unistd.h>    /* close */

#endif

#include <cassert>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace tui {
    // only the Esc character
    constexpr const char ESC = '\x1B';
    // Control Sequence Introducer actually == "ESC[" almost everything starts with this
    constexpr const char* CSI = "\x1B[";

    // Variadic template function to concatenate any number of arguments
    template <typename... Args> std::string concat(Args&&... args) {
        std::ostringstream oss;
        (void)std::initializer_list<int>{
            (oss << std::forward<Args>(args), 0)...}; // Using initializer_list for fold-like behavior
        return oss.str();
    }

// Control Sequence Introducer
#define csi(...) std::cout << concat("\x1B[", __VA_ARGS__);
// function using Control Sequence Introducer
#define csi_fn(name, ...)                                                                                              \
    inline void name() { csi(__VA_ARGS__) }

// ANSII Escape Sequnce
#define esc(...) std::cout << concat("\x1B", __VA_ARGS__);
// function using ANSII Escape Sequence
#define esc_fn(name, ...)                                                                                              \
    inline void name() { esc(__VA_ARGS__) }

#ifdef _WIN32 // windows
// basic setup of windows terminal handling
#define win_setup()                                                                                                    \
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);                                                                    \
    if (hStdin == INVALID_HANDLE_VALUE) {                                                                              \
        std::cerr << "Error getting the standard input handle." << std::endl;                                          \
        exit(1);                                                                                                       \
    }                                                                                                                  \
    DWORD mode;                                                                                                        \
    if (!GetConsoleMode(hStdin, &mode)) {                                                                              \
        std::cerr << "Error getting the console mode." << std::endl;                                                   \
        exit(1);                                                                                                       \
    }
#endif

    inline void enable_raw_mode() {
#ifdef _WIN32 // windows
        win_setup();

        DWORD newMode = mode;
        newMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);

        if (!SetConsoleMode(hStdin, newMode)) {
            std::cerr << "Error setting the console to raw mode." << std::endl;
            exit(1);
        }
#else // not windows
        system("stty raw");
        system("stty -echo");

        // struct termios term {};
        // if (tcgetattr(STDIN_FILENO, &term) == -1) {
        //     std::cerr << "Error getting terminal attributes." << std::endl;
        //     exit(1);
        // }

        // struct termios raw = term;
        // raw.c_lflag &= ~(ICANON | ECHO);

        // if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        //     std::cerr << "Error setting terminal to raw mode." << std::endl;
        //     exit(1);
        // }
#endif
    }

    inline void disable_raw_mode() {
#ifdef _WIN32 // windows
        win_setup();

        // Restore original mode
        mode |= (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
        if (!SetConsoleMode(hStdin, mode)) {
            std::cerr << "Error restoring the console mode." << std::endl;
            exit(1);
        }
#else // not windows
        system("stty -raw");
        system("stty echo");

        // struct termios term {};
        // if (tcgetattr(STDIN_FILENO, &term) == -1) {
        //     std::cerr << "Error getting terminal attributes." << std::endl;
        //     exit(1);
        // }

        // // Restore original attributes
        // term.c_lflag |= (ICANON | ECHO);

        // if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1) {
        //     std::cerr << "Error restoring terminal mode." << std::endl;
        //     exit(1);
        // }
#endif
    }

#undef win_setup

    namespace cursor {
// template for moving cursor
// moves cursor `n` times to `dir`
#define move_n(dir, ch)                                                                                                \
    inline void dir(unsigned n = 1) { csi(n, '#', ch); }

        move_n(up, 'A');
        move_n(down, 'B');
        move_n(right, 'C');
        move_n(left, 'D');

        // moves cursor one row up, scrolling if needed
        esc_fn(up_n_scroll, 'M');
        // moves cursor to beginning of next line, `n` rows down
        move_n(next_line, 'E');
        // moves cursor to beginning of previous line, `n` rows up
        move_n(prev_line, 'F');

        // moves cursor to home position (0;0)
        csi_fn(home, 'H');
        // moves cursor to (`row`;`col`), both start at 1
        inline void set_position(unsigned row, unsigned col) { csi(row, ';', col, 'H'); }
        // moves cursor to column `n`
        move_n(to_column, 'G');
#undef move_n

        // save cursor position
        esc_fn(save, '7');
        // restore previously saved cursor position
        esc_fn(restore, '8');

        // set visibility
        inline void visible(bool visible) { csi("?25", (visible ? 'h' : 'l')); }

        // tell the terminal to check where the cursor is
        csi_fn(query_position, "6n");

        // (rows;cols)
        inline std::pair<unsigned, unsigned> get_position() {
            query_position();
            std::flush(std::cout);
            // Read the response: ESC [ rows ; cols R
            char ch = 0;
            unsigned rows = 0;
            unsigned cols = 0;
            if (std::cin.get(ch) && ch == ESC && std::cin.get(ch) && ch == '[') {
                std::cin >> rows;
                std::cin.get(ch); // skip the ';'
                std::cin >> cols;
                std::cin.ignore(); // skip the 'R'
            }

            return {rows, cols};
        }
    } // namespace cursor

    namespace screen {
        // clears
        csi_fn(clear, "2J");
        csi_fn(clear_line, "2K");
        csi_fn(clear_line_right, "K");

        // erases
        inline void erase_in_line(unsigned n = 0) { csi(n, 'K'); }
        inline void erase_in_display(unsigned n = 0) { csi(n, 'J'); }
        csi_fn(erase_saved_lines, "3J");

        csi_fn(save_screen, "?47h");
        csi_fn(restore_screen, "?47l");

        inline void alternative_buffer(bool enable) { csi("?1049", (enable ? 'h' : 'l')); }

        inline void scroll_up(unsigned n = 1) { csi(n, 'S'); }
        inline void scroll_down(unsigned n = 1) { csi(n, 'T'); }

        // get the size of the terminal: (rows;cols)/(y;x)
        inline std::pair<unsigned, unsigned> size() {
#ifdef _WIN32
            HANDLE console;
            CONSOLE_SCREEN_BUFFER_INFO info;
            short rows;
            short columns;
            /* Create a handle to the console screen. */
            console = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                  OPEN_EXISTING, 0, NULL);
            if (console == INVALID_HANDLE_VALUE)
                return {0, 0};

            /* Calculate the size of the console window. */
            if (GetConsoleScreenBufferInfo(console, &info) == 0)
                return {0, 0};
            CloseHandle(console);
            columns = info.srWindow.Right - info.srWindow.Left + 1;
            rows = info.srWindow.Bottom - info.srWindow.Top + 1;

            return {rows, columns};
#else
            struct winsize ws;
            int fd;

            /* Open the controlling terminal. */
            fd = open("/dev/tty", O_RDWR);
            if (fd < 0)
                err(1, "/dev/tty");

            /* Get window size of terminal. */
            if (ioctl(fd, TIOCGWINSZ, &ws) < 0)
                err(1, "/dev/tty");

            // printf("%d rows by %d columns\n", ws.ws_row, ws.ws_col);
            // printf("(%d by %d pixels)\n", ws.ws_xpixel, ws.ws_ypixel);
            close(fd);
            return {ws.ws_row, ws.ws_col};
#endif
        }
    } // namespace screen

    namespace text {
        namespace style {
            enum Style {
                reset = 0,
                bold = 1,
                dim = 2,
                italic = 3,
                underline = 4,
                blink = 5,
                inverted = 7,
                invisible = 8,
                strikethrough = 9,
            };

            inline std::string style(const Style& style) { return concat(CSI, static_cast<unsigned>(style), 'm'); }

// generate for cout << STYLE_style(); eg.: cout << bold_style();
#define stylize(STYLE)                                                                                                 \
    inline std::string STYLE##_style() { return style(STYLE); }
// generate for cout << STYLE_style(text); eg.: cout << bold_style(text);
#define stylize_text(STYLE)                                                                                            \
    inline std::string STYLE##_style(const std::string& text) { return concat(style(STYLE), text, reset_style()); }    \
    inline std::string STYLE##_style(const char* text) { return concat(style(STYLE), text, reset_style()); }
// generate all
#define make_stylizer(STYLE) stylize(STYLE) stylize_text(STYLE)

            stylize(reset);
            inline std::string style_and_reset(const Style& st, const std::string& text) {
                return concat(style(st), text, reset_style());
            }

            make_stylizer(bold);
            make_stylizer(dim);
            make_stylizer(italic);
            make_stylizer(underline);
            make_stylizer(blink);
            make_stylizer(inverted);
            make_stylizer(invisible);
            make_stylizer(strikethrough);
#undef make_stylizer
#undef stylize_text
#undef stylize

            // printf '\e]8;;http://example.com\e\\This is a link\e]8;;\e\\\n'
            // printf 'ESC]8;;{link}ESC\\{text}ESC]8;;ESC\\'
            inline std::string link(const std::string& link, const std::string& text) {
                return concat(ESC, "]8;;", link, ESC, "\\", text, ESC, "]8;;", ESC, "\\");
            }
            inline std::string link(const char* link, const char* text) {
                return concat(ESC, "]8;;", link, ESC, "\\", text, ESC, "]8;;", ESC, "\\");
            }

        } // namespace style

        namespace color {
            enum Color {
                black = 0,
                red,
                green,
                yellow,
                blue,
                magenta,
                cyan,
                white,
                basic,
            };

            // fg or bg
            inline std::string colorizer(const Color& c, bool fg) {
                return concat(CSI, (fg ? '3' : '4'), static_cast<unsigned>(c), 'm');
            }

// generate for cout << COLOR_{fg, bg}();
#define colorize(COLOR)                                                                                                \
    inline std::string COLOR##_fg() { return colorizer(COLOR, true); }                                                 \
    inline std::string COLOR##_bg() { return colorizer(COLOR, false); }

// generate for cout << COLOR_{fg, bg}(text);
#define colorize_text(COLOR)                                                                                           \
    inline std::string COLOR##_fg(const std::string& text) {                                                           \
        return concat(colorizer(COLOR, true), text, style::reset_style());                                             \
    }                                                                                                                  \
    inline std::string COLOR##_fg(const char* text) {                                                                  \
        return concat(colorizer(COLOR, true), text, style::reset_style());                                             \
    }                                                                                                                  \
    inline std::string COLOR##_bg(const std::string& text) {                                                           \
        return concat(colorizer(COLOR, false), text, style::reset_style());                                            \
    }                                                                                                                  \
    inline std::string COLOR##_bg(const char* text) {                                                                  \
        return concat(colorizer(COLOR, false), text, style::reset_style());                                            \
    }

// generate all
#define make_colorizer(COLOR) colorize(COLOR) colorize_text(COLOR)
            make_colorizer(black);
            make_colorizer(red);
            make_colorizer(green);
            make_colorizer(yellow);
            make_colorizer(blue);
            make_colorizer(magenta);
            make_colorizer(cyan);
            make_colorizer(white);
            make_colorizer(basic);
#undef make_colorizer
#undef colorize_text
#undef colorize

            // r, g, b values have to be valid:  [0;255]
            inline std::string rgb(unsigned r, unsigned g, unsigned b, bool fg) {
                assert(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255);
                return concat(CSI, (fg ? '3' : '4'), "8;2;", r, ';', g, ';', b, 'm');
            }
            inline std::string rgb(unsigned r, unsigned g, unsigned b, bool fg, const std::string& text) {
                return concat(rgb(r, g, b, fg), text, style::reset_style());
            }

        } // namespace color

    } // namespace text

    class tui_string : public std::string {
      public:
        tui_string() = default;
        template <typename T> tui_string(T s) : std::string(concat(s)) {}
        tui_string(const char* s) : std::string(s) {}
        tui_string(const std::string& s) : std::string(s) {}

// generate this.STYLE(): eg this.italic()
#define make_style(STYLE)                                                                                              \
    inline tui_string STYLE() const { return text::style::STYLE##_style(*this); }

        make_style(bold);
        make_style(dim);
        make_style(italic);
        make_style(underline);
        make_style(blink);
        make_style(inverted);
        make_style(invisible);
        make_style(strikethrough);
#undef make_style

// generate this.COLOR(): eg this.red()
#define make_color(COLOR)                                                                                              \
    inline tui_string COLOR() const { return text::color::COLOR##_fg(*this); }                                         \
    inline tui_string on_##COLOR() const { return text::color::COLOR##_bg(*this); }

        make_color(black);
        make_color(red);
        make_color(green);
        make_color(yellow);
        make_color(blue);
        make_color(magenta);
        make_color(cyan);
        make_color(white);
        make_color(basic);
#undef make_color

        inline tui_string link(const char* link) { return text::style::link(link, *this); }
        inline tui_string rgb(unsigned r, unsigned g, unsigned b) const {
            return text::color::rgb(r, g, b, true, *this);
        }
        inline tui_string on_rgb(unsigned r, unsigned g, unsigned b) const {
            return text::color::rgb(r, g, b, false, *this);
        }
    };

    // void handle_resize(int /*sig*/) { screen::clear(); }
    using fn_ptr = void (*)(int);
    // NOTE: does not work on Windows
    // needs a void function, that takes an int
    // function pointer: `void function_name(int sig) { stuff }`
    inline void set_up_resize(fn_ptr handle_resize) {
#ifdef _WIN32
        // no such thing that's this easy
#else
        // Register the signal handler for SIGWINCH
        struct sigaction sa {};
        sa.sa_handler = handle_resize;
        sa.sa_flags = SA_RESTART; // Restart functions if interrupted by handler
        sigaction(SIGWINCH, &sa, nullptr);
#endif
    }

    inline void init_term(bool enable_cursor) {
#ifdef _WIN32
        SetConsoleOutputCP(65001); // use utf-8
#endif
        tui::enable_raw_mode();
        tui::cursor::visible(enable_cursor);
        tui::screen::save_screen();
        tui::screen::alternative_buffer(true);
        tui::screen::clear();
        tui::cursor::home();
    }
    inline void reset_term() {
        tui::disable_raw_mode();
        tui::screen::alternative_buffer(false);
        tui::screen::restore_screen();
        tui::cursor::visible(true);
    }

    namespace input {
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
            void read(const char& ch) { *this = Input::read_from(ch); }
            static Input read_from(const char& ch) {
                auto pee = 0;
                if /* mostly weird stuff, like: 'Ű' */ (ch < 0) {
                    std::cin.ignore();
                    return Input::from_special(SpecKey::None);
                }

                if (ch >= 32 && ch <= 126) { // <char>
                    return Input::from_char(ch);
                } else if (ch >= 1 && ch <= 26) { // Ctrl<char>
                    return Input::from_special(static_cast<SpecKey>(ch));
                }
                switch (ch) {
                case SpecKey::Backspace:
                    return Input::from_special(static_cast<SpecKey>(ch));
                case SpecKey::Esc: {
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
                        case SpecKey::F1:
                        case SpecKey::F2:
                        case SpecKey::F3:
                        case SpecKey::F4:
                            return Input::from_special(static_cast<SpecKey>(sus));
                        default:
                            std::cin.ignore(3);
                            break;
                        }
                        break;
                    }
                    default:
                        return Input::from_special(SpecKey::Esc);
                    }
                default:
                    break;
                }
                }
                std::ofstream logf("tui_input.log", std::ios::app);
                logf << "\nerror: an unknown character:\n\t- code: '" << static_cast<int>(ch) << "'\n\t- display: '"
                     << ch << "'\n\n";
                logf.close();
                return Input::from_special(SpecKey::None);
            }
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
    } // namespace input
} // namespace tui
