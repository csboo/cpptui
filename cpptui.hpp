// tui.h
#pragma once
#ifndef TUI_H
#define TUI_H

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
        inline void up(int n = 1) { std::cout << ESC << n << "#A"; }
        // moves cursor down `n` rows
        inline void down(int n = 1) { std::cout << ESC << n << "#B"; }
        // moves cursor right `n` columns
        inline void right(int n = 1) { std::cout << ESC << n << "#C"; }
        // moves cursor left `n` columns
        inline void left(int n = 1) { std::cout << ESC << n << "#D"; }

        // moves cursor one row up, scrolling if needed
        inline void up_n_scroll() { std::cout << PURE_ESC << 'M'; }
        // moves cursor to beginning of next line, `n` rows down
        inline void next_line(int n = 1) { std::cout << ESC << n << "#E"; }
        // moves cursor to beginning of previous line, `n` rows up
        inline void prev_line(int n = 1) { std::cout << ESC << n << "#F"; }

        // moves cursor to home position (0;0)
        inline void home() { std::cout << ESC << 'H'; }
        // moves cursor to (`row`;`col`)
        inline void move_to(int row, int col) { std::cout << ESC << row << ';' << col << 'H'; }
        // moves cursor to column `n`
        inline void to_column(int n) { std::cout << ESC << n << "#G"; }

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
        inline void erase_in_line(int n = 0) { std::cout << ESC << n << 'K'; }
        inline void erase_in_display(int n = 0) { std::cout << ESC << n << 'J'; }
        inline void erase_saved_lines() { std::cout << ESC << "3J"; }

        inline void save_screen() { std::cout << ESC << "?47h"; }
        inline void restore_screen() { std::cout << ESC << "?47l"; }

        inline void alternative_buffer(bool enable) { std::cout << ESC << "?1049" << (enable ? 'h' : 'l'); }

        inline void scroll_up(int n = 1) { std::cout << ESC << n << 'S'; }
        inline void scroll_down(int n = 1) { std::cout << ESC << n << 'T'; }
    } // namespace screen

    namespace text {
        namespace style {
            enum Style {
                reset = 0,
                bold,
                dim,
                italic,
                underline,
                blink,
                inverted,
                invisible,
                strikethrough,
            };

            inline std::string style(const Style& style) {
                return std::string(ESC) + std::to_string(static_cast<unsigned>(style)) + "m";
            }

#define stylize(STYLE)                                                                                                 \
    inline std::string STYLE##_style() { return style(STYLE); }
#define stylize_text(STYLE)                                                                                            \
    inline std::string STYLE##_style(const std::string& text) { return style(STYLE) + text + reset_style(); }          \
    inline std::string STYLE##_style(const char* text) { return style(STYLE) + text + reset_style(); }
#define make_stylizer(STYLE) stylize(STYLE) stylize_text(STYLE)

            stylize(reset);
            inline std::string style_and_reset(const Style& st, const std::string& s) {
                return style(st) + s + reset_style();
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
                return std::string(ESC) + (fg ? '3' : '4') + std::to_string(static_cast<unsigned>(c)) + "m";
            }

#define colorize(COLOR)                                                                                                \
    inline std::string COLOR##_fg() { return colorizer(COLOR, true); }                                                 \
    inline std::string COLOR##_bg() { return colorizer(COLOR, false); }
#define colorize_text(COLOR)                                                                                           \
    inline std::string COLOR##_fg(const std::string& text) {                                                           \
        return colorizer(COLOR, true) + text + style::reset_style();                                                   \
    }                                                                                                                  \
    inline std::string COLOR##_fg(const char* text) { return colorizer(COLOR, true) + text + style::reset_style(); }   \
    inline std::string COLOR##_bg(const std::string& text) {                                                           \
        return colorizer(COLOR, false) + text + style::reset_style();                                                  \
    }                                                                                                                  \
    inline std::string COLOR##_bg(const char* text) { return colorizer(COLOR, false) + text + style::reset_style(); }
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

            inline std::string rgb(int r, int g, int b, bool fg) {
                std::ostringstream oss;
                oss << ESC << (fg ? '3' : '4') << "8;2;" << r << ";" << g << ";" << b << "m";
                return oss.str();
            }
            inline std::string rgb(int r, int g, int b, bool fg, const std::string& text) {
                std::ostringstream oss;
                oss << ESC << (fg ? '3' : '4') << "8;2;" << r << ";" << g << ";" << b << "m" << text
                    << style::reset_style();
                return oss.str();
            }

        } // namespace color

    } // namespace text
    class tui_string : public std::string {
      public:
        tui_string() = default;
        tui_string(const char* s) : std::string(s) {}
        tui_string(const std::string& s) : std::string(s) {}

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

#define make_color(COLOR)                                                                                              \
    inline tui_string COLOR() const { return text::color::COLOR##_fg(*this); }                                         \
    inline tui_string COLOR##_bg() const { return text::color::COLOR##_bg(*this); }

        make_color(black);
        make_color(red);
        make_color(green);
        make_color(yellow);
        make_color(blue);
        make_color(magenta);
        make_color(cyan);
        make_color(white);
        make_color(basic);

        inline std::string rgb_fg(int r, int g, int b) const { return text::color::rgb(r, g, b, true, *this); }
        inline std::string rgb_bg(int r, int g, int b) const { return text::color::rgb(r, g, b, false, *this); }
    };
} // namespace tui

#endif // TUI_H
