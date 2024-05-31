#include "../tui.hpp"
#include <cassert>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

void text() {
    auto cur_pos = tui::cursor::get_position();
    // std::cout << tui::tui_string("Hello").on_rgb(255, 0, 0).rgb(0, 0, 255) << " "
    //           << tui::tui_string("World?").blue().italic().strikethrough() << " "
    //           << tui::tui_string("Ain't no way!").on_red().bold().black() << " "
    //           << "cursor at: " << cur_pos.first << ";" << cur_pos.second << " "
    //           << tui::tui_string("it's more like I'm on the").underline() << " "
    //           << tui::tui_string("Moon").link("https://www.moon.com/").underline().blue() << "\r\n";
    std::cout << tui::tui_string("RESIZED").red();
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
        std::cout << tui::tui_string(x / 100).on_red().black();
    } else if (x % 10 == 0) {
        std::cout << tui::tui_string(x / 10 % 10).on_blue().black();
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

std::unordered_map<kind, std::vector<std::string>> kinds() {
    return {{empty, {" ", " ", " ", " ", " ", " "}},
            {basic, {"┌", "┐", "└", "┘", "│", "─"}},
            {bold, {"┏", "┓", "┗", "┛", "┃", "━"}},
            {rounded, {"╭", "╮", "╰", "╯", "│", "─"}}};
}

// start.row ------------ start.col
// |                              |
// |                              |
// |                              |
// |                              |
// |                              |
// |                              |
// |                              |
// end.row ---------------- end.col
void box(coord start, coord end, kind with) {
    assert(start.row <= end.row && start.col <= end.col);

    auto draw = kinds()[with];

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

// catch special characters, that might mess up things
void filter_chars(char ch) {
    if (ch < 0) {
        std::cin.ignore();
        ch = 0;
    } else if (ch == 27 && std::cin.peek() == 91) {
        tui::cursor::set_position(40, 140 - 2);
        std::cin.ignore();
        auto sus = std::cin.get();
        std::cout << "oh! an arrow? ";
        switch (sus) {
        case 65:
            std::cout << "up   ";
            break;
        case 66:
            std::cout << "down ";
            break;
        case 67:
            std::cout << "right";
            break;
        case 68:
            std::cout << "left ";
            break;
        default:
            std::cout << "NO!  ";
            std::cin.get();
            std::cin.get();
            std::cin.ignore();
            break;
        }
    }
}
void run() {
    auto screen_size = tui::screen::size();
    auto screen = coord{screen_size.first, screen_size.second};

    auto msg = tui::tui_string("Szia Csongi!");
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

    char x = 0;
    while (x != 'q') {

        screen_size = tui::screen::size();
        screen = coord{screen_size.first, screen_size.second};
        msg_start = coord{screen.row / 2, static_cast<unsigned int>((screen.col / 2) - msg_len / 2)};
        msg_end = coord{screen.row / 2, static_cast<unsigned int>((screen.col / 2) + msg_len / 2)};

        counter_box(coord{1, 1}, screen);
        if (x == 9 /* tab */) {
            if (current_box == boxes.size() - 1) {
                current_box = 0;
            } else {
                current_box++;
            }
        } else if (x == 'j') {
            auto* cb = &boxes[current_box];
            box(cb->first, cb->second, empty);
            cb->first.row++;
            cb->second.row++;
        } else if (x == 'k') {
            auto* cb = &boxes[current_box];
            box(cb->first, cb->second, empty);
            cb->first.row--;
            cb->second.row--;
        } else if (x == 'h') {
            auto* cb = &boxes[current_box];
            box(cb->first, cb->second, empty);
            cb->first.col--;
            cb->second.col--;
        } else if (x == 'l') {
            auto* cb = &boxes[current_box];
            box(cb->first, cb->second, empty);
            cb->first.col++;
            cb->second.col++;
        } else if (x == '-') {
            auto* cb = &boxes[current_box];
            box(cb->first, cb->second, empty);
            cb->first.row++;
            cb->first.col++;

            cb->second.row--;
            cb->second.col--;
        } else if (x == '+') {
            auto* cb = &boxes[current_box];
            box(cb->first, cb->second, empty);
            cb->first.row--;
            cb->first.col--;

            cb->second.row++;
            cb->second.col++;
        }

        for (auto item : boxes) {
            if (item == boxes[current_box]) {
                std::cout << tui::text::color::cyan_fg();
                box(item.first, item.second, rounded);
                std::cout << tui::text::style::reset_style();
            } else {
                box(item.first, item.second, basic);
            }
        }

        tui::cursor::set_position(msg_start.row, msg_start.col);
        std::cout << msg.bold().italic().inverted().blue();

        tui::cursor::set_position(screen.row / 3 * 2, screen.col / 3 * 2);
        std::cout << tui::tui_string("tui.hpp").bold().blue().link("https://github.com/csboo/cpptui").on_magenta();
        // tui::cursor::set_position(39, 140);
        // std::cout << "\\──────┘";
        // std::cout << "████████";

        // std::thread(box, std::make_pair(msg_start.first - 1, msg_start.second - 1),
        //             std::make_pair(msg_end.first + 1, msg_end.second), rounded)
        //     .detach();

        // 120fps
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
        std::cin.get(x);
        filter_chars(x);
    }
}

void handle_resize(int /*sig*/) {
    // tui::cursor::get_position();
    tui::screen::clear();
    // tui::cursor::home();
    // []() { std::cout << "RESIZED"; };
    // text();
    // std::cout << "RESIZED";
}

int main() {
    tui::init_term(false);
    tui::set_up_resize(handle_resize);

    // tui::cursor::set_position(2, 20);
    // for (auto i = 0; i < 40; ++i) {
    //     text();
    // }
    try {
        run();
    } catch (...) {
        tui::reset_term();
        std::cout << "ran into a problem";
        return 1;
    }

    // tui::cursor::set_position(screen.first / 2 - 1, (screen.second / 2) - msg_len / 2);
    // tui::cursor::move_to(2, 20);
    // tui::cursor::right(40);
    // tui::cursor::to_column(40);
    // std::cout << tui::tui_string("Press `Ret` to quit...").yellow();

    tui::reset_term();
}
