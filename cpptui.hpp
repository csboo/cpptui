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
        inline std::string save() { return "\033[s"; }
        inline std::string restore() { return "\033[u"; }
        inline std::string visible(bool visible) { return visible ? "\033[?25h" : "\033[?25l"; }
    }

    namespace screen {
        inline std::string clear() { return "\033[2J"; }
        inline std::string clearLine() { return "\033[2K"; }
        inline std::string alternativeBuffer(bool enable) { return enable ? "\033[?1049h" : "\033[?1049l"; }
    }

    namespace text {
        inline constexpr const char* reset() { return "\033[0m"; }
        inline constexpr const char* bold() { return "\033[1m"; }
        inline std::string bold(const std::string& text) { 
            return "\033[1m" + text + "\033[0m"; 
        }
        inline constexpr const char* underline() { return "\033[4m"; }
        inline std::string underline(const std::string& text) { 
            return "\033[4m" + text + "\033[0m"; 
        }
        inline constexpr const char* reversed() { return "\033[7m"; }
        inline std::string reversed(const std::string& text) { 
            return "\033[7m" + text + "\033[0m"; 
        }
    }

    namespace set_fg {
        inline constexpr const char* black() { return "\033[30m"; }
        inline constexpr const char* red() { return "\033[31m"; }
        inline constexpr const char* green() { return "\033[32m"; }
        inline constexpr const char* yellow() { return "\033[33m"; }
        inline constexpr const char* blue() { return "\033[34m"; }
        inline constexpr const char* magenta() { return "\033[35m"; }
        inline constexpr const char* cyan() { return "\033[36m"; }
        inline constexpr const char* white() { return "\033[37m"; }
    }

    namespace set_bg {
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

#endif // TUI_H
