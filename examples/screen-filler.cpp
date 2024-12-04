#include "../coords.hpp"
#include "../tui.hpp"
#include <thread>

using namespace tui::input;

struct State {
    Input input;
    bool quit = false;
} state;

const Coord TOP_LEFT = Coord::origin();
const tui::string MSG = tui::string(" ");

void read_char() {
    while (state.input != 'q' && state.input != SpecKey::CtrlC) {
        state.input = Input::read();
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    state.quit = true;
}

void run() {
    auto screen_size = Coord();
    auto prev_screen_size = screen_size;

    do {
        prev_screen_size = screen_size;
        screen_size = Coord::screen_size();
        if (prev_screen_size == screen_size) {
            continue;
        }

        for (unsigned row = TOP_LEFT.row; row <= screen_size.row; ++row) {
            for (unsigned col = TOP_LEFT.col; col <= screen_size.col; ++col) {
                Coord(row, col).print(MSG.on_white());
            }
        }

        auto top_right = TOP_LEFT.with_col(screen_size.col);
        auto bottom_right = screen_size;
        auto bottom_left = screen_size.with_col(TOP_LEFT.col);

        auto mod_mid_hor = screen_size.col % 2;
        auto even_mid_hor = mod_mid_hor == 0;

        auto mod_mid_ver = screen_size.row % 2;
        auto even_mid_ver = mod_mid_ver == 0;

        auto mid_hor = screen_size.col / 2 + mod_mid_hor;
        auto mid_ver = screen_size.row / 2 + mod_mid_ver;
        auto mid_mid = Coord(mid_ver, mid_hor);

        TOP_LEFT.print(MSG.on_black());
        top_right.print(MSG.on_cyan());

        mid_mid.print(MSG.on_green());

        if (even_mid_hor) {
            mid_mid.with_col(mid_hor + 1).print(MSG.on_green());
        }
        if (even_mid_ver) {
            auto left_lower = mid_mid.with_row(mid_ver + 1);
            left_lower.print(MSG.on_green());
            if (even_mid_hor) {
                left_lower.with_col(mid_hor + 1).print(MSG.on_green());
            }
        }

        bottom_left.print(MSG.on_magenta());
        // std::cout
        //     << tui::string(tui::concat(" [size: (", screen_size.row, ";", screen_size.col, ")]")).on_white().black();
        bottom_right.print(MSG.on_red());

        // NOTE: it's necessary!
        std::cout.flush();
    } while (!state.quit);
}

int main() {
    tui::init();

    auto reader_thread = std::thread(read_char);

    run();

    reader_thread.join();

    tui::reset();
}
