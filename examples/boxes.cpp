#include "../tui.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <thread>
#include <utility>
#include <vector>

using namespace tui::input;

struct Coord {
    unsigned row;
    unsigned col;

    Coord(const unsigned& row, const unsigned& col) : row{row}, col{col} {}
    // Coord(const Coord& copy) : row{copy.row}, col{copy.col} {}
    Coord(const std::pair<unsigned, unsigned>& copy) : row{copy.first}, col{copy.second} {}

    bool operator==(const Coord& other) const { return (this->row == other.row && this->col == other.col); }
    // bool operator!=(const Coord& other) const { return !(this == &other); }
    // Coord operator=(std::pair<unsigned, unsigned> other) {
    //     this->row = other.first;
    //     this->col = other.second;
    //     return *this;
    // }
};

using Box = std::pair<Coord, Coord>;

struct AppState {
    Input input;
    bool quit = false;
    bool new_input = true;
    Coord size = tui::screen::size();
} state;

// make `x` be good for `counter_box`
void count(const uint64_t& x) {
    unsigned r = 0;
    if (x % 100 == 0) {
        auto print = std::to_string(x / 100);
        std::cout << tui::string(print[0]).on_red().black();
    } else if (x % 10 == 0) {
        auto print = std::to_string(x / 10 % 10);
        std::cout << tui::string(print[0]).on_blue().black();
    } else {
        auto print = std::to_string(x);
        std::cout << print.back();
    }
}

void counter_box(Coord start, Coord end) {
    assert(start.row <= end.row && start.col <= end.col);

    // do rows
    // from top to down
    for (auto row = start.row + 1; row < end.row; ++row) {
        // left row
        tui::cursor::set_position(row, start.col);

        count(row);
        // right row
        tui::cursor::set_position(row, end.col);
        count(row);
    }

    // do columns
    // top left
    tui::cursor::set_position(start.row, start.col);
    for (auto col = start.col; col <= end.col; ++col) {
        count(col);
    }
    // bottom left
    tui::cursor::set_position(end.row, start.col);
    for (auto col = start.col; col <= end.col; ++col) {
        count(col);
    }
}

enum Kind {
    Empty,
    Basic,
    Bold,
    Rounded,
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
void draw_box(Box box, Kind with) {
    auto start = box.first;
    auto end = box.second;
    assert(start.row <= end.row && start.col <= end.col);

    auto draw = KINDS[with];

    // do rows
    for (auto row = start.row + 1; row < end.row; ++row) {
        // left row
        tui::cursor::set_position(row, start.col);
        std::cout << draw[4];
        // right row
        tui::cursor::set_position(row, end.col);
        std::cout << draw[4];
    }

    // do columns
    // top left
    tui::cursor::set_position(start.row, start.col);
    std::cout << draw[0];
    for (auto i = start.col + 1; i < end.col; ++i) {
        std::cout << draw[5];
    }
    // top right
    std::cout << draw[1];
    // bottom left
    tui::cursor::set_position(end.row, start.col);
    std::cout << draw[2];
    for (auto i = start.col + 1; i < end.col; ++i) {
        std::cout << draw[5];
    }
    // bottom right
    std::cout << draw[3];
}

void handle_keys(std::vector<Box>& boxes, int& cnt_box_ix) {
    auto* cnt_box = &boxes[cnt_box_ix];
    if (state.input == 'n' || state.input == SpecKey::Tab) {
        if (cnt_box_ix++ == boxes.size() - 1) {
            cnt_box_ix = 0;
        }
    } else if (state.input == 'p' /* || state.input == SpecKey::ShiftTab */) {
        if (cnt_box_ix-- == 0) {
            cnt_box_ix = boxes.size() - 1;
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
void run() {
    const auto msg = tui::string("Szia Csongi!");
    auto msg_coord = [msg](bool left) {
        return Coord{state.size.row / 2 + 1,
                     static_cast<unsigned int>((state.size.col / 2) + (left ? -msg.size() : +msg.size()) / 2)};
    };

    std::vector<Box> boxes = {
        {{6, 6}, {12, 12}},
        {{8, 10}, {19, 41}},
        {
            {20, 8},
            {30, 12},
        },
    };

    auto cnt_box_ix = 0;

    while (state.input != 'q' && state.input != SpecKey::CtrlC) {
        if (!state.new_input) {
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

        tui::cursor::set_position(msg_start.row, msg_start.col);
        std::cout << msg.bold().italic().inverted().blue();

        tui::cursor::set_position(state.size.row / 3 * 2, state.size.col / 3 * 2);
        std::cout << tui::string("tui.hpp").blue().link("https://github.com/csboo/cpptui").on_magenta();

        state.new_input = false;

        std::cout.flush();
        // 120fps
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    state.quit = true;
}

void handle_read() {
    while (!state.quit) {
        state.input = Input::read();
        state.new_input = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    std::cout << "reader thread done\n";
}

void handle_resize() {
    Coord prev = state.size;
    while (!state.quit) {
        state.size = tui::screen::size();
        if (prev.col != state.size.col || prev.row != state.size.row) {
            tui::screen::clear();
            state.new_input = true;
        }
        prev = state.size;
        std::this_thread::sleep_for(std::chrono::milliseconds(256));
    }
    std::cout << "resizer thread done\n";
}

int main() {
    tui::init();

    std::thread reader(handle_read);
    std::thread resizer(handle_resize);

    try {
        run();
    } catch (...) {
        tui::reset();
        std::cout << "ran into a problem";
        return 1;
    }

    Coord boxcord(state.size.row / 2, state.size.col / 2);
    tui::string msg = "Press any key to quit.";
    unsigned msghalf = msg.size() / 2;
    Coord msgcord(boxcord.row, boxcord.col - msghalf);
    Box box = {{boxcord.row - 2, boxcord.col - msghalf - 2}, {boxcord.row + 2, boxcord.col + msghalf + 2}};

    draw_box(box, Kind::Bold);
    tui::cursor::set_position(msgcord.row, msgcord.col);
    std::cout << msg.italic().magenta().on_black().underline();

    resizer.join();
    reader.join();
    tui::reset();

    return 0;
}
