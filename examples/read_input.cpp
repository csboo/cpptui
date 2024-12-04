#include "../tui.hpp"

using namespace tui::input;

// get's called on terminall resize
void clear(int /*sig*/) {
    tui::screen::clear(); // clears screen
    tui::cursor::home();  // sets the cursor to the top left corner of the screen
    // no need to flush `cout`, as `'\n'` does it (well, yeah. don't ask me why though)
    std::cout << "NOTE: resized.\r\n";
}

int main() {
    tui::init();
    // NOTE: doesn't work on windows
    // on terminal resize, we use our `clear()` function
    tui::set_up_resize(clear);

    try {
        Input input;
        do {
            input = Input::read();                        // convert it into an `Input`
            std::cout << "-\tinput: " << input << "\r\n"; // see how easy it is to print an `Input`
        } while (input != SpecKey::CtrlC);
    } catch (...) { // NOTE: probably won't happen, but it's good to be careful
        tui::reset();
        std::cerr << "\nunknown error\n\n";
        return 1;
    }

    tui::reset();
    return 0;
}
