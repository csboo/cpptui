#include "../coords.hpp"
#include "../tui.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

using namespace tui::input;

using Box = std::pair<Coord, Coord>;

static struct AppState {
    Input input;
    bool quit = false;
    bool new_input = true;
    Coord size = Coord::screen_size();
} state;

// make `x` be good for `counter_box`
static std::string count(const uint64_t& x) {
    // unsigned r = 0;
    std::string print;
    if (x % 100 == 0) {
        print = std::to_string(x / 100);
        print = tui::string(print[0]).on_red().black();
    } else if (x % 10 == 0) {
        print = std::to_string(x / 10 % 10);
        print = tui::string(print[0]).on_blue().black();
    } else {
        print = std::to_string(x);
        print = print.back();
    }
    return print;
}

static void counter_box(Coord start, Coord end) {
    assert(start.row <= end.row && start.col <= end.col);

    // do rows
    // from top to down
    for (auto row = start.row + 1; row < end.row; ++row) {
        // left row
        start.with_row(row).print(count(row));
        // right row
        end.with_row(row).print(count(row));
    }

    // do columns
    // top left
    start.set_cursor();
    for (auto col = start.col; col <= end.col; ++col) {
        std::cout << count(col);
    }
    // bottom left
    end.with_col(start.col).set_cursor();
    for (auto col = start.col; col <= end.col; ++col) {
        std::cout << count(col);
    }
}

enum class Kind : std::uint8_t{
    Empty = 0,
    Basic = 1,
    Bold = 2,
    Rounded = 3,
};

const std::vector<std::vector<std::string>> KINDS = {{{" ", " ", " ", " ", " ", " "}},
                                                     {{"┌", "┐", "└", "┘", "│", "─"}},
                                                     {{"┏", "┓", "┗", "┛", "┃", "━"}},
                                                     {{"╭", "╮", "╰", "╯", "│", "─"}}};

// start.row ------------ start.col
// |                              |
// |                              |
// |                              |
// |                              |
// |                              |
// |                              |
// |                              |
// end.row ---------------- end.col
static void draw_box(Box box, Kind with) {
    auto start = box.first;
    auto end = box.second;
    assert(start.row <= end.row && start.col <= end.col);

    const auto& draw = KINDS.at(static_cast<size_t>(with));

    // do rows
    for (auto row = start.row + 1; row < end.row; ++row) {
        // left row
        start.with_row(row).print(draw[4]);
        // right row
        end.with_row(row).print(draw[4]);
    }

    // do columns
    // top left
    start.print(draw[0]);
    for (auto i = start.col + 1; i < end.col; ++i) {
        std::cout << draw[5];
    }
    // top right
    std::cout << draw[1];
    // bottom left
    end.with_col(start.col).print(draw[2]);
    for (auto i = start.col + 1; i < end.col; ++i) {
        std::cout << draw[5];
    }
    // bottom right
    std::cout << draw[3];
}

static void handle_keys(std::vector<Box>& boxes, unsigned& cnt_box_ix) {
    auto* cnt_box = &boxes[cnt_box_ix];
    if (state.input == 'n' || state.input == SpecKey::Tab) {
        if (cnt_box_ix++ == boxes.size() - 1) {
            cnt_box_ix = 0;
        }
    } else if (state.input == 'p' /* || state.input == SpecKey::ShiftTab */) {
        if (cnt_box_ix-- == 0) {
            cnt_box_ix = static_cast<int>(boxes.size()) - 1;
        }
    } else if (state.input == 'j' || state.input == Arrow::Down) {
        draw_box(*cnt_box, Kind::Empty);
        cnt_box->first.row++;
        cnt_box->second.row++;
    } else if (state.input == 'k' || state.input == Arrow::Up) {
        draw_box(*cnt_box, Kind::Empty);
        cnt_box->first.row--;
        cnt_box->second.row--;
    } else if (state.input == 'h' || state.input == Arrow::Left) {
        draw_box(*cnt_box, Kind::Empty);
        cnt_box->first.col--;
        cnt_box->second.col--;
    } else if (state.input == 'l' || state.input == Arrow::Right) {
        draw_box(*cnt_box, Kind::Empty);
        cnt_box->first.col++;
        cnt_box->second.col++;
    } else if (state.input == '-') {
        draw_box(*cnt_box, Kind::Empty);
        cnt_box->first.row++;
        cnt_box->first.col++;

        cnt_box->second.row--;
        cnt_box->second.col--;
    } else if (state.input == '+') {
        draw_box(*cnt_box, Kind::Empty);
        cnt_box->first.row--;
        cnt_box->first.col--;

        cnt_box->second.row++;
        cnt_box->second.col++;
    } else if (state.input == 'd' || state.input == SpecKey::Backspace) {
        draw_box(*cnt_box, Kind::Empty);
        boxes.erase(boxes.begin() + cnt_box_ix);
    }
}
static void run() {
    const auto msg = tui::string("Szia Csongi!");
    auto msg_coord = [msg](bool left) {
        return Coord{(state.size.row / 2) + 1,
                     static_cast<unsigned int>((state.size.col / 2) + ((left ? -msg.size() : +msg.size()) / 2))};
    };

    std::vector<Box> boxes = {
        {{6, 6}, {12, 12}},
        {{8, 10}, {19, 41}},
        {
            {20, 8},
            {30, 12},
        },
    };

    unsigned cnt_box_ix = 0;
    Coord prev_size;

    do {
        state.size = Coord::screen_size();
        if (prev_size != state.size) {
            prev_size = state.size;
            tui::screen::clear();
        } else if (!state.new_input) {
            continue; // if there's no new input, don't draw anything
        }
        counter_box({1, 1}, state.size);
        handle_keys(boxes, cnt_box_ix);
        auto msg_start = msg_coord(true);
        auto msg_end = msg_coord(false);
        Box msg_box = {{msg_start.row - 1, msg_start.col - 1}, {msg_end.row + 1, msg_end.col}};
        if (!std::any_of(boxes.begin(), boxes.end(), [msg_box](Box item) { return item == msg_box; })) {
            boxes.push_back(msg_box);
        }

        for (auto box : boxes) {
            if (box == boxes[cnt_box_ix]) {
                std::cout << tui::text::color::cyan_fg();
                draw_box(box, Kind::Rounded);
                std::cout << tui::text::style::reset_style();
            } else {
                draw_box(box, Kind::Basic);
            }
        }

        msg_start.print(msg.bold().italic().inverted().blue());

        Coord(state.size.row / 3 * 2, state.size.col / 3 * 2)
            .print(tui::string("tui.hpp").blue().link("https://github.com/csboo/cpptui").on_magenta());

        state.new_input = false;

        std::cout.flush();
        // 120fps
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    } while (!state.quit);
}

static void handle_read() {
    while (state.input != 'q' && state.input != SpecKey::CtrlC) {
        state.input = Input::read();
        state.new_input = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    state.quit = true;
}

int main() {
    tui::init();

    std::thread reader(handle_read);

    run();

    reader.join();

    tui::reset();

    return 0;
}
