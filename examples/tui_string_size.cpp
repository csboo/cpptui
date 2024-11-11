#include "../tui.hpp"
#include <fstream>

int main() {
    std::string orig = "0123456789";
    auto styled = tui::string(orig).on_green().italic().inverted().rgb(100, 100, 100).link("https://codeberg.org");

    std::ofstream fout("test.txt");
    fout << "\"" << orig << "\"" << orig.size() << "\n";
    fout << "\"" << styled << "\"" << styled.size() << "\n";
    fout.close();

    std::cout << "hello";
    return 0;
}
