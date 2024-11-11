#include "../tui.hpp"
#include <cassert>
#include <chrono>
#include <thread>
#include <utility>
#include <vector>

using tui::input::Arrow;
using tui::input::Input;
using tui::input::SpecKey;

std::pair<unsigned int, unsigned int> screen_size;

void text() {
    auto cur_pos = tui::cursor::get_position();
    // std::cout << tui::string("Hello").on_rgb(255, 0, 0).rgb(0, 0, 255) << " "
    //           << tui::string("World?").blue().italic().strikethrough() << " "
    //           << tui::string("Ain't no way!").on_red().bold().black() << " "
    //           << "cursor at: " << cur_pos.first << ";" << cur_pos.second << " "
    //           << tui::string("it's more like I'm on the").underline() << " "
    //           << tui::string("Moon").link("https://www.moon.com/").underline().blue() << "\r\n";
    std::cout << tui::string("RESIZED").red();
}

struct coord {
    unsigned row;
    unsigned col;

    bool operator==(const coord& other) const { return (this->row == other.row && this->col == other.col); }
};

// make `x` be good for `counter_box`
void count(const unsigned long long& x) {
    unsigned r = 0;
    if (x % 100 == 0) {
        std::cout << tui::string(x / 100).on_red().black();
    } else if (x % 10 == 0) {
        std::cout << tui::string(x / 10 % 10).on_blue().black();
    } else {
        std::cout << x % 10;
    }
}

void counter_box(coord start, coord end) {
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

enum kind {
    empty,
    basic,
    bold,
    rounded,
};

const std::vector<std::vector<std::string>> kinds = {{{" ", " ", " ", " ", " ", " "}},
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
void draw_box(coord start, coord end, kind with) {
    assert(start.row <= end.row && start.col <= end.col);

    auto draw = kinds[with];

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

void run() {
    auto screen = coord{screen_size.first, screen_size.second};

    auto msg = tui::string("Szia Csongi!");
    unsigned msg_len = msg.size();
    auto msg_start = coord{screen.row / 2, static_cast<unsigned int>((screen.col / 2) - msg_len / 2)};
    auto msg_end = coord{screen.row / 2, static_cast<unsigned int>((screen.col / 2) + msg_len / 2)};

    std::vector<std::pair<coord, coord>> boxes = {
        {{6, 6}, {12, 12}},
        {{8, 10}, {19, 41}},
        {
            {20, 8},
            {30, 12},
        },
        {{msg_start.row - 1, msg_start.col - 1}, {msg_end.row + 1, msg_end.col}}};

    auto current_box = 0;

    char ch = 0;
    Input x;
    while (x != 'q' && x != SpecKey::CtrlC) {
        screen = coord{screen_size.first, screen_size.second};
        msg_start = coord{screen.row / 2, static_cast<unsigned int>((screen.col / 2) - msg_len / 2)};
        msg_end = coord{screen.row / 2, static_cast<unsigned int>((screen.col / 2) + msg_len / 2)};

        counter_box(coord{1, 1}, screen);
        if (x == SpecKey::Tab) {
            if (current_box == boxes.size() - 1) {
                current_box = 0;
            } else {
                current_box++;
            }
        } else if (x == 'j' || x == Arrow::Down) {
            auto* cb = &boxes[current_box];
            draw_box(cb->first, cb->second, empty);
            cb->first.row++;
            cb->second.row++;
        } else if (x == 'k' || x == Arrow::Up) {
            auto* cb = &boxes[current_box];
            draw_box(cb->first, cb->second, empty);
            cb->first.row--;
            cb->second.row--;
        } else if (x == 'h' || x == Arrow::Left) {
            auto* cb = &boxes[current_box];
            draw_box(cb->first, cb->second, empty);
            cb->first.col--;
            cb->second.col--;
        } else if (x == 'l' || x == Arrow::Right) {
            auto* cb = &boxes[current_box];
            draw_box(cb->first, cb->second, empty);
            cb->first.col++;
            cb->second.col++;
        } else if (x == '-') {
            auto* cb = &boxes[current_box];
            draw_box(cb->first, cb->second, empty);
            cb->first.row++;
            cb->first.col++;

            cb->second.row--;
            cb->second.col--;
        } else if (x == '+') {
            auto* cb = &boxes[current_box];
            draw_box(cb->first, cb->second, empty);
            cb->first.row--;
            cb->first.col--;

            cb->second.row++;
            cb->second.col++;
        }

        for (auto item : boxes) {
            if (item == boxes[current_box]) {
                std::cout << tui::text::color::cyan_fg();
                draw_box(item.first, item.second, rounded);
                std::cout << tui::text::style::reset_style();
            } else {
                draw_box(item.first, item.second, basic);
            }
        }

        tui::cursor::set_position(msg_start.row, msg_start.col);
        std::cout << msg.bold().italic().inverted().blue();

        tui::cursor::set_position(screen.row / 3 * 2, screen.col / 3 * 2);
        std::cout << tui::string("tui.hpp").bold().blue().link("https://github.com/csboo/cpptui").on_magenta();
        // tui::cursor::set_position(39, 140);
        // std::cout << "\\──────┘";
        // std::cout << "████████";

        // std::thread(box, std::make_pair(msg_start.first - 1, msg_start.second - 1),
        //             std::make_pair(msg_end.first + 1, msg_end.second), rounded)
        //     .detach();

        // 120fps
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
        std::cin.get(ch);
        x.read(ch);
    }
}

// NOTE: doesn't work on Windows
void handle_resize(int /*sig*/) {
    screen_size = tui::screen::size();
    // tui::cursor::get_position();
    tui::screen::clear();
    tui::cursor::home();
    // std::cout << "\n";
    std::cout.flush();
    // []() { std::cout << "RESIZED"; };
    // text();
    // std::cout << "RESIZED";
}

int main() {
    tui::init(false);
    // NOTE: doesn't work on Windows
    tui::set_up_resize(handle_resize);
    screen_size = tui::screen::size();

    // tui::cursor::set_position(2, 20);
    // for (auto i = 0; i < 40; ++i) {
    //     text();
    // }
    try {
        run();
    } catch (...) {
        tui::reset();
        std::cout << "ran into a problem";
        return 1;
    }

    // tui::cursor::set_position(screen.first / 2 - 1, (screen.second / 2) - msg_len / 2);
    // tui::cursor::move_to(2, 20);
    // tui::cursor::right(40);
    // tui::cursor::to_column(40);
    // std::cout << tui::string("Press `Ret` to quit...").yellow();

    tui::reset();
}
