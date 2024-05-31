#include "../tui.hpp"
#include <iostream>

int main() {
    tui::init_term(false);

    try {
        char input = 0;
        while (input != 'q') {
            std::cout << '\'' << input << '\'' << tui::tui_string("\tchar:").italic() << " \'"
                      << tui::tui_string(static_cast<int>(input)).bold() << "\'\r\n";
            std::cin.get(input);
        }
    } catch (...) {
        std::cout << "baj.";
        tui::reset_term();
        return 1;
    }

    tui::reset_term();
    return 0;
}
