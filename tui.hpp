// tui.hpp
#pragma once
#ifndef TUI_H
#define TUI_H

#include <cassert>
#include <csignal>
#include <iostream>
#include <sstream>
#include <string>

namespace tui {
    // only the Esc character
    constexpr const char PURE_ESC = '\x1B';
    // actually == "ESC[" almost everything starts with this
    constexpr const char* const ESC = "\x1B[";
    namespace cursor {
        // moves cursor up `n` rows
        inline void up(unsigned n = 1) { std::cout << ESC << n << "#A"; }
        // moves cursor down `n` rows
        inline void down(unsigned n = 1) { std::cout << ESC << n << "#B"; }
        // moves cursor right `n` columns
        inline void right(unsigned n = 1) { std::cout << ESC << n << "#C"; }
        // moves cursor left `n` columns
        inline void left(unsigned n = 1) { std::cout << ESC << n << "#D"; }

        // moves cursor one row up, scrolling if needed
        inline void up_n_scroll() { std::cout << PURE_ESC << 'M'; }
        // moves cursor to beginning of next line, `n` rows down
        inline void next_line(unsigned n = 1) { std::cout << ESC << n << "#E"; }
        // moves cursor to beginning of previous line, `n` rows up
        inline void prev_line(unsigned n = 1) { std::cout << ESC << n << "#F"; }

        // moves cursor to home position (0;0)
        inline void home() { std::cout << ESC << 'H'; }
        // moves cursor to (`row`;`col`)
        inline void move_to(unsigned row, unsigned col) { std::cout << ESC << row << ';' << col << 'H'; }
        // moves cursor to column `n`
        inline void to_column(unsigned n) { std::cout << ESC << n << "#G"; }

        // save cursor position
        inline void save() { std::cout << PURE_ESC << '7'; }
        // restore previously saved cursor position
        inline void restore() { std::cout << PURE_ESC << '8'; }

        // set visibility
        inline void visible(bool visible) { std::cout << ESC << "?25" << (visible ? 'h' : 'l'); }

        // check where the cursor is
        inline void request_position() { std::cout << ESC << "6n"; }
    } // namespace cursor

    namespace screen {
        // clears
        inline void clear() { std::cout << ESC << "2J"; }
        inline void clear_line() { std::cout << ESC << "2K"; }
        inline void clear_line_right() { std::cout << ESC << "K"; }

        // erases
        inline void erase_in_line(unsigned n = 0) { std::cout << ESC << n << 'K'; }
        inline void erase_in_display(unsigned n = 0) { std::cout << ESC << n << 'J'; }
        inline void erase_saved_lines() { std::cout << ESC << "3J"; }

        inline void save_screen() { std::cout << ESC << "?47h"; }
        inline void restore_screen() { std::cout << ESC << "?47l"; }

        inline void alternative_buffer(bool enable) { std::cout << ESC << "?1049" << (enable ? 'h' : 'l'); }

        inline void scroll_up(unsigned n = 1) { std::cout << ESC << n << 'S'; }
        inline void scroll_down(unsigned n = 1) { std::cout << ESC << n << 'T'; }
    } // namespace screen

    namespace text {
        // Helper function to stream a single argument
        template <typename T> void stream_arg(std::ostringstream& oss, T&& arg) { oss << std::forward<T>(arg); }
        // Variadic template function to concatenate any number of arguments
        template <typename... Args> std::string concat(Args&&... args) {
            std::ostringstream oss;
            (void)std::initializer_list<int>{
                (stream_arg(oss, std::forward<Args>(args)), 0)...}; // Using initializer_list for fold-like behavior
            return oss.str();
        }

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

            inline std::string style(const Style& style) { return concat(ESC, static_cast<unsigned>(style), 'm'); }

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
            inline std::string style_and_reset(const Style& st, const std::string& s) {
                return concat(style(st), s, reset_style());
            }

            make_stylizer(bold);
            make_stylizer(dim);
            make_stylizer(italic);
            make_stylizer(underline);
            make_stylizer(blink);
            make_stylizer(inverted);
            make_stylizer(invisible);
            make_stylizer(strikethrough);
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
                return concat(ESC, (fg ? '3' : '4'), static_cast<unsigned>(c), 'm');
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

            // r, g, b values have to be valid:  [0;255]
            inline std::string rgb(unsigned r, unsigned g, unsigned b, bool fg) {
                assert(r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255);
                return concat(ESC, (fg ? '3' : '4'), "8;2;", r, ';', g, ';', b, 'm');
            }
            inline std::string rgb(unsigned r, unsigned g, unsigned b, bool fg, const std::string& text) {
                return concat(rgb(r, g, b, fg), text, style::reset_style());
            }

        } // namespace color

    } // namespace text
    class tui_string : public std::string {
      public:
        tui_string() = default;
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

        inline tui_string rgb(unsigned r, unsigned g, unsigned b) const {
            return text::color::rgb(r, g, b, true, *this);
        }
        inline tui_string on_rgb(unsigned r, unsigned g, unsigned b) const {
            return text::color::rgb(r, g, b, false, *this);
        }
    };
} // namespace tui

#endif // TUI_H
