#include "../tui.hpp"
#include <iostream>

int main() {
    tui::init(false);

    try {
        char input = 0;
        while (input != 'q') {
            std::cout << '\'' << input << '\'' << tui::string("\tchar:").italic() << " \'"
                      << tui::string(static_cast<int>(input)).bold() << "\'\r\n";
            std::cin.get(input);
        }
    } catch (...) {
        std::cout << "baj.";
        tui::reset();
        return 1;
    }

    tui::reset();
    return 0;
}
