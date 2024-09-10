#include "../input.hpp"
#include "../tui.hpp"

// get's called on terminall resize
void clear(int sig) {
    tui::screen::clear(); // clears screen
    tui::cursor::home();  // sets the cursor to the top left corner of the screen
    // no need to flush `cout`, as `'\n'` does it (well, yeah. don't ask me why though)
    std::cout << "NOTE: resized.\r\n";
}

int main() {
    tui::init_term(false);
    // NOTE: doesn't work on windows
    // on terminal resize, we use our `clear()` function
    tui::set_up_resize(clear);

    try {
        char ch = 0;
        Input input;
        do {
            std::cin.get(ch);                                                  // read into a character
            input.read(ch);                                                    // convert it into an `Input`
            std::cout << "ch:\t'" << ch << "'\t-\tinput: " << input << "\r\n"; // see how easy it is to print an `Input`
        } while (input != Special::CtrlC);
    } catch (...) { // NOTE: probably won't happen, but it's good to be careful
        tui::reset_term();
        std::cerr << "\nunknown error\n\n";
        return 1;
    }

    tui::reset_term();
    return 0;
}
