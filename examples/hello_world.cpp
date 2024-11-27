#include "../tui.hpp"
#include <utility> // for std::pair

using tui::input::Input;
using tui::input::SpecKey;

int main() {
    tui::init(false);                       // alternate buffer, raw mode, no cursor
    auto screen_size = tui::screen::size(); // initially set

    const tui::string msg = "Hello Bello, TUI!"; // stylable extension of `std::string`
    const tui::string note = "press q or Ctrl+C to quit";

    Input input;                                      // this will handle special stuff like arrows, ctrl+c, ...
    while (input != SpecKey::CtrlC && input != 'q') { // main loop
        // NOTE: use `.size()` before styling, as it'd be completely crazy after applying styles
        // `msg` will be in the middle of the screen
        tui::cursor::set_position(screen_size.first / 2 - 1, screen_size.second / 2 - (msg.size() / 2));
        std::cout << msg.blue().link("https://github.com/csboo/cpptui").bold().underline();

        // `msg` will be in the middle of the screen
        tui::cursor::set_position(screen_size.first / 2 + 1, screen_size.second / 2 - (note.size() / 2));
        std::cout << note.on_rgb(106, 150, 137).rgb(148, 105, 117).italic().dim();

        std::cout.flush();
        input = Input::read(); // read into an `Input`
    }

    tui::reset(); // restore to where we came from
    return 0;
}
