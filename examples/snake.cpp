#include "../coords.hpp"
#include "../tui.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace tui::input;

// this is how the apple/food will be displayed
const tui::string APPLE_TEXT = tui::string('@').red().bold();
// this is the default duration a frame lives for in ms, it's 23.8 fps
const std::chrono::milliseconds SLEEP_MS = std::chrono::milliseconds(42);
const std::chrono::milliseconds ADD_MS = std::chrono::milliseconds(1);
const std::chrono::milliseconds MAX_MS = std::chrono::milliseconds(std::numeric_limits<unsigned>::infinity());
// initial size/lenght of the snake: at the game start
const unsigned INIT_LEN = 5;

// direction
enum Dir {
    Up = 0,
    Down,
    Left,
    Right,
    None,
};
Dir opposite(const Dir& dir) {
    switch (dir) {
    case Dir::Up:
        return Dir::Down;
    case Dir::Down:
        return Dir::Up;
    case Dir::Left:
        return Dir::Right;
    case Dir::Right:
        return Dir::Left;
    case Dir::None:
        break;
    }
    return Dir::None;
}
Dir from_input(const Input& input, const Dir& dir = Dir::Right) {
    switch (input.ch) {
    case 'k':
    case 'w':
        return Dir::Up;
    case 'j':
    case 's':
        return Dir::Down;
    case 'l':
    case 'd':
        return Dir::Right;
    case 'h':
    case 'a':
        return Dir::Left;
    default:
        break;
    }
    switch (input.arrow) {
    case Arrow::Up:
        return Dir::Up;
    case Arrow::Down:
        return Dir::Down;
    case Arrow::Right:
        return Dir::Right;
    case Arrow::Left:
        return Dir::Left;
    default:
        break;
    }
    return dir;
}

std::string to_string(const Dir& dir) {
    switch (dir) {
    case Dir::Up:
        return "↑"; // alt: ^
    case Dir::Down:
        return "↓"; // alt: ˇ
    case Dir::Left:
        return "←"; // alt: <
    case Dir::Right:
        return "→"; // alt: >
    case Dir::None:
        break;
    }
    return "X";
}

Dir meets_at(const Coord& lhs, const Coord& rhs, const Coord& screen_size) {
    int row_diff = static_cast<int>(lhs.row) - static_cast<int>(rhs.row);
    int col_diff = static_cast<int>(lhs.col) - static_cast<int>(rhs.col);

    // we set both row and col to x-1 as it's needed :D
    auto teleport = Coord{screen_size.row - 1, screen_size.col - 1};

    if (row_diff == 1 || teleport.row == -row_diff) {
        return Dir::Up;
    }
    if (row_diff == -1 || teleport.row == row_diff) {
        return Dir::Down;
    }
    if (col_diff == 1 || teleport.col == -col_diff) {
        return Dir::Left;
    }
    if (col_diff == -1 || teleport.col == col_diff) {
        return Dir::Right;
    }
    return Dir::None;
}

using Snake = std::vector<Coord>;

std::pair<Dir, Dir> neighbours(const Snake& snake, const unsigned& idx, const Coord& screen_size) {
    // std::ofstream fout("babkelme.log", std::ios::app);
    auto coord = snake[idx];

    Coord prev{};
    if (snake.size() > 1) {
        prev = snake.at(idx - 1);
        if (prev == coord && snake.size() > 2) {
            prev = snake.at(idx - 2);
        }
    }
    Coord next{};
    if (idx < snake.size() - 1) {
        next = snake.at(idx + 1);
    }

    Dir first = meets_at(coord, prev, screen_size);
    Dir second = meets_at(coord, next, screen_size);

    return {first, second};
}

std::string draw(const std::pair<Dir, Dir>& nb) {
    // rounded:  {"╭", "╮", "╰", "╯", "│", "─"}

    // this is where in Rust we'd use `match` and be happy
    if (((nb.first == Dir::Up || nb.first == Dir::Down) && (nb.second == Dir::Down || nb.second == Dir::Up)) ||
        ((nb.first == Dir::None && (nb.second == Dir::Down || nb.second == Dir::Up))) ||
        ((nb.second == Dir::None && (nb.first == Dir::Down || nb.first == Dir::Up)))) {
        return "│";
    }
    if (((nb.first == Dir::Left || nb.first == Dir::Right) && (nb.second == Dir::Right || nb.second == Dir::Left)) ||
        (nb.first == Dir::None && (nb.second == Dir::Left || nb.second == Dir::Right)) ||
        (nb.second == Dir::None && (nb.first == Dir::Left || nb.first == Dir::Right))) {
        return "─";
    }
    if ((nb.first == Dir::Left && nb.second == Dir::Down) || (nb.second == Dir::Left && nb.first == Dir::Down)) {
        return "╮";
    }
    if ((nb.first == Dir::Right && nb.second == Dir::Down) || (nb.first == Dir::Down && nb.second == Dir::Right)) {
        return "╭";
    }
    if ((nb.first == Dir::Left && nb.second == Dir::Up) || (nb.second == Dir::Left && nb.first == Dir::Up)) {
        return "╯";
    }
    if ((nb.first == Dir::Right && nb.second == Dir::Up) || (nb.second == Dir::Right && nb.first == Dir::Up)) {
        return "╰";
    }
    return "X";
}

void handle_movement(const Dir& dir, Coord& coord, const Coord& ss) {
    switch (dir) {
    case Dir::Up:
        // move to the `Down` side of the screen if would go too far `Up`
        if (coord.row - 1 == 0) {
            coord.row = ss.row;
            break;
        }
        coord.row--;
        break;
    case Dir::Down:
        // move to the `Up`per side the screen if would go too far `Down`
        if (coord.row + 1 > ss.row) {
            coord.row = 1;
            break;
        }
        coord.row++;
        break;
    case Dir::Left:
        // move to the `Right` side the screen if would go too far `Left`
        if (coord.col - 1 == 0) {
            coord.col = ss.col;
            break;
        }
        coord.col--;
        break;
    case Dir::Right:
        // move to the `Left` side the screen if would go too far `Right`
        if (coord.col + 1 > ss.col) {
            coord.col = 1;
            break;
        }
        coord.col++;
        break;
    case Dir::None:
        break;
    }
}
void move(Snake& snake, const Dir& dir, const Coord& ss) {
    // delete the last one off the screen by overwriting it with a space
    snake.back().print(' ');
    auto old_snake = snake;
    for (auto i = 1; i < snake.size(); ++i) {
        snake.at(i) = old_snake.at(i - 1);
    }

    handle_movement(dir, snake.front(), ss);
}

bool snake_contains(const Snake& snake, const Coord& coord, const unsigned& skip = 0) {
    return std::find_if(std::begin(snake) + skip, snake.end(), [&](const Coord& item) { return item == coord; }) !=
           snake.end();
}

struct App {
    Coord screen_size = Coord::screen_size();
    Snake snake = App::default_snake();
    Coord apple = Coord::random(this->screen_size);
    Dir dir = Dir::Right;
    Input input;
    bool quit = false;
    bool new_input = true;

    void update_apple() { this->apple = Coord::random(this->screen_size); }
    static Snake default_snake() {
        auto mid = Coord::screen_size() / 2;
        Snake snake;
        for (auto i = 0; i < INIT_LEN; ++i) {
            snake.push_back(mid.with_col(mid.col - i));
        }
        return snake;
    }

} app;

void handle_read() {
    while (!app.quit && app.input != 'q' && app.input != 'Q' && app.input != SpecKey::CtrlC &&
           app.input != SpecKey::CtrlD && app.input != SpecKey::CtrlZ) {
        app.input = Input::read();
        app.new_input = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    app.quit = true;
    std::cout << "reader thread done\n";
}

void run() {
    auto score_count = Coord{2, 4};
    score_count.print(tui::string(tui::concat("score: ", app.snake.size() - INIT_LEN)).green().italic());

    // // get which direction the snake shall move to, if character is invalid, don't change: use `dir`
    // auto prev_dir = dir;
    // dir = from_input(ch, dir);
    // // don't try to break your neck if you may!
    // if (prev_dir == opposite(dir)) {
    //     dir = prev_dir;
    // }

    app.apple.print(APPLE_TEXT);
    do {
        if (!app.new_input) {
            continue;
        }

        auto prev_dir = app.dir;
        app.dir = from_input(app.input, prev_dir);
        if (prev_dir == opposite(app.dir)) {
            app.dir = prev_dir;
        }

        // and move it correspondly
        move(app.snake, app.dir, app.screen_size);

        // die if wanna eat itself
        if (snake_contains(app.snake, app.snake.front(), 1)) {
            app.quit = true;
            return;
        }

        // snake ate apple, we need a new one!
        if (app.snake.front() == app.apple) {
            std::vector<Coord> non_snake;
            for (unsigned i = 1; i < app.screen_size.row; ++i) {
                for (unsigned j = 1; j < app.screen_size.col; ++j) {
                    if (!snake_contains(app.snake, Coord{i, j})) {
                        non_snake.emplace_back(i, j);
                    }
                }
            }
            if (non_snake.empty()) {
                app.quit = true;
                return;
            }
            std::mt19937 mt{std::random_device{}()};
            std::uniform_int_distribution<unsigned> gen_idx(0, non_snake.size() - 1);
            unsigned idx = gen_idx(mt);

            app.apple = non_snake.at(idx);
            // duplicate the last element of the `snake`, next round it'll be smoothed out.
            // assert(snake.size() + 1 == snake.previous_size())
            app.snake.push_back(app.snake.back());
            score_count.print(tui::string(tui::concat("score: ", app.snake.size() - INIT_LEN)).green().italic());
            app.apple.print(APPLE_TEXT);
        }

        // print non-head parts of snake, but only first 2
        for (auto i = 1; i < ((app.snake.size() == 1) ? 1 : 2); ++i) {
            auto nb = neighbours(app.snake, i, app.screen_size);
            app.snake[i].print(tui::string(draw(nb)).blue());
        }
        // print head
        app.snake.front().print(tui::string(to_string(app.dir)).blue());

        std::cout.flush();
        auto sleep_mul = (app.dir == Dir::Left || app.dir == Dir::Right) ? 1. : 1.5;
        auto sleep_dur =
            SLEEP_MS + (10 > app.snake.size() ? -(ADD_MS * 10) + ADD_MS * static_cast<unsigned>(app.snake.size())
                                              : ADD_MS * static_cast<unsigned>(app.snake.size()));
        // sleep, if moving vertically: more
        std::this_thread::sleep_for(sleep_dur * sleep_mul);
    } while (!app.quit);
}

int main() {
    try {
        tui::init();

        std::thread reader(handle_read);

        run();

        auto len = app.snake.size() - INIT_LEN;

        tui::reset();
        if (static_cast<unsigned>(len) == app.screen_size.row * app.screen_size.col) {
            std::cout << "Congrats, you won!\n";
        } else {
            std::cout << "You died/quit at " << len << "\n";
        }
        reader.join();

    } catch (...) {
        tui::reset();
        std::cerr << "unknown error occured\n";
        return 1;
    }

    tui::reset();

    return 0;
}
