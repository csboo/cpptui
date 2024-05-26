// tui.h
#ifndef TUI_H
#define TUI_H

#include <string>
#include <sstream>

namespace tui {
    namespace cursor {
        inline std::string up(int n = 1) { 
            std::ostringstream oss; 
            oss << "\033[" << n << "A"; 
            return oss.str(); 
        }
        inline std::string down(int n = 1) { 
            std::ostringstream oss; 
            oss << "\033[" << n << "B"; 
            return oss.str(); 
        }
        inline std::string forward(int n = 1) { 
            std::ostringstream oss; 
            oss << "\033[" << n << "C"; 
            return oss.str(); 
        }
        inline std::string backward(int n = 1) { 
            std::ostringstream oss; 
            oss << "\033[" << n << "D"; 
            return oss.str(); 
        }
        inline std::string position(int row, int col) { 
            std::ostringstream oss; 
            oss << "\033[" << row << ";" << col << "H"; 
            return oss.str(); 
        }
        inline std::string position(const std::string& row, const std::string& col) {
            return "\033[" + row + ";" + col + "f";
        }
        inline std::string save() { return "\033[s"; }
        inline std::string restore() { return "\033[u"; }
        inline std::string visible(bool visible) { return visible ? "\033[?25h" : "\033[?25l"; }
        inline std::string home() { return "\033[H"; }
        inline std::string column(int n) { 
            std::ostringstream oss; 
            oss << "\033[" << n << "G"; 
            return oss.str(); 
        }
        inline std::string nextLine(int n = 1) {
            std::ostringstream oss;
            oss << "\033[" << n << "E";
            return oss.str();
        }
        inline std::string prevLine(int n = 1) {
            std::ostringstream oss;
            oss << "\033[" << n << "F";
            return oss.str();
        }
        inline std::string requestPosition() { return "\033[6n"; }
        inline std::string cursorUp() { return "\033M"; }
        inline std::string saveDEC() { return "\0337"; }
        inline std::string restoreDEC() { return "\0338"; }
    }

    namespace screen {
        inline std::string clear() { return "\033[2J"; }
        inline std::string clearLine() { return "\033[2K"; }
        inline std::string clearLineRight() { return "\033[K"; }
        inline std::string alternativeBuffer(bool enable) { return enable ? "\033[?1049h" : "\033[?1049l"; }
        inline std::string scrollUp(int n = 1) { 
            std::ostringstream oss; 
            oss << "\033[" << n << "S"; 
            return oss.str(); 
        }
        inline std::string scrollDown(int n = 1) { 
            std::ostringstream oss; 
            oss << "\033[" << n << "T"; 
            return oss.str(); 
        }
        inline std::string eraseInLine(int n = 0) {
            std::ostringstream oss;
            oss << "\033[" << n << "K";
            return oss.str();
        }
        inline std::string eraseInDisplay(int n = 0) {
            std::ostringstream oss;
            oss << "\033[" << n << "J";
            return oss.str();
        }
        inline std::string eraseSavedLines() { return "\033[3J"; }
    }

    namespace text {
        inline constexpr const char* reset() { return "\033[0m"; }
        inline constexpr const char* bold() { return "\033[1m"; }
        inline std::string bold(const std::string& text) { 
            return "\033[1m" + text + "\033[0m"; 
        }
        inline constexpr const char* dim() { return "\033[2m"; }
        inline std::string dim(const std::string& text) { 
            return "\033[2m" + text + "\033[0m"; 
        }
        inline constexpr const char* italic() { return "\033[3m"; }
        inline std::string italic(const std::string& text) { 
            return "\033[3m" + text + "\033[0m"; 
        }
        inline constexpr const char* underline() { return "\033[4m"; }
        inline std::string underline(const std::string& text) { 
            return "\033[4m" + text + "\033[0m"; 
        }
        inline constexpr const char* blink() { return "\033[5m"; }
        inline std::string blink(const std::string& text) { 
            return "\033[5m" + text + "\033[0m"; 
        }
        inline constexpr const char* inverted() { return "\033[7m"; }
        inline std::string inverted(const std::string& text) { 
            return "\033[7m" + text + "\033[0m"; 
        }
        inline constexpr const char* invisible() { return "\033[8m"; }
        inline std::string invisible(const std::string& text) { 
            return "\033[8m" + text + "\033[0m"; 
        }
        inline constexpr const char* strikethrough() { return "\033[9m"; }
        inline std::string strikethrough(const std::string& text) { 
            return "\033[9m" + text + "\033[0m"; 
        }
   
        inline constexpr const char* black() { return "\033[30m"; }
        inline std::string black(std::string text) { 
            return "\033[30m" + text + "\033[0m"; 
        }
        inline constexpr const char* red() { return "\033[31m"; }
        inline std::string red(std::string text) { 
            return "\033[31m" + text + "\033[0m"; 
        }
        inline constexpr const char* green() { return "\033[32m"; }
        inline std::string green(std::string text) { 
            return "\033[32m" + text + "\033[0m"; 
        }
        inline constexpr const char* yellow() { return "\033[33m"; }
        inline std::string yellow(std::string text) { 
            return "\033[33m" + text + "\033[0m"; 
        }
        inline constexpr const char* blue() { return "\033[34m"; }
        inline std::string blue(std::string text) { 
            return "\033[34m" + text + "\033[0m"; 
        }
        inline constexpr const char* magenta() { return "\033[35m"; }
        inline std::string magenta(std::string text) { 
            return "\033[35m" + text + "\033[0m"; 
        }
        inline constexpr const char* cyan() { return "\033[36m"; }
        inline std::string cyan(std::string text) { 
            return "\033[36m" + text + "\033[0m"; 
        }
        inline constexpr const char* white() { return "\033[37m"; }
        inline std::string white(std::string text) { 
            return "\033[37m" + text + "\033[0m"; 
        }

        namespace bg {
            inline constexpr const char* black() { return "\033[40m"; }
            inline constexpr const char* red() { return "\033[41m"; }
            inline constexpr const char* green() { return "\033[42m"; }
            inline constexpr const char* yellow() { return "\033[43m"; }
            inline constexpr const char* blue() { return "\033[44m"; }
            inline constexpr const char* magenta() { return "\033[45m"; }
            inline constexpr const char* cyan() { return "\033[46m"; }
            inline constexpr const char* white() { return "\033[47m"; }
        }
    }

    class tui_string : public std::string {
    public:
        tui_string() : std::string() {}
        tui_string(const char* s) : std::string(s) {}
        tui_string(const std::string& s) : std::string(s) {}

        inline tui_string bold() const {
            return text::bold(*this);
        }

        inline tui_string dim() const {
            return text::dim(*this);
        }

        inline tui_string italic() const {
            return text::italic(*this);
        }

        inline tui_string underline() const {
            return text::underline(*this);
        }

        inline tui_string blink() const {
            return text::blink(*this);
        }

        inline tui_string inverted() const {
            return text::inverted(*this);
        }

        inline tui_string invisible() const {
            return text::invisible(*this);
        }

        inline tui_string strikethrough() const {
            return text::strikethrough(*this);
        }

        inline tui_string black() const {
            return text::black(*this);
        }

        inline tui_string red() const {
            return text::red(*this);
        }

        inline tui_string green() const {
            return text::green(*this);
        }

        inline tui_string yellow() const {
            return text::yellow(*this);
        }

        inline tui_string blue() const {
            return text::blue(*this);
        }

        inline tui_string magenta() const {
            return text::magenta(*this);
        }

        inline tui_string cyan() const {
            return text::cyan(*this);
        }

        inline tui_string white() const {
            return text::white(*this);
        }

        inline tui_string bg_black() const {
            return std::string(text::bg::black()) + *this + text::reset();
        }

        inline tui_string bg_red() const {
            return std::string(text::bg::red()) + *this + text::reset();
        }

        inline tui_string bg_green() const {
            return std::string(text::bg::green()) + *this + text::reset();
        }

        inline tui_string bg_yellow() const {
            return std::string(text::bg::yellow()) + *this + text::reset();
        }

        inline tui_string bg_blue() const {
            return std::string(text::bg::blue()) + *this + text::reset();
        }

        inline tui_string bg_magenta() const {
            return std::string(text::bg::magenta()) + *this + text::reset();
        }

        inline tui_string bg_cyan() const {
            return std::string(text::bg::cyan()) + *this + text::reset();
        }

        inline tui_string bg_white() const {
            return std::string(text::bg::white()) + *this + text::reset();
        }
    };
}

#endif // TUI_H
