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
Dir from_char(const char& ch, const Dir& dir = Dir::Right) {
    switch (ch) {
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
        // TODO: make arrows usable again
        if (ch < 0) {
            std::cin.ignore();
        } else if (ch == 27 && std::cin.peek() == 91) {
            std::cin.ignore();
            auto sus = std::cin.get();
            switch (sus) {
            case 65:
                return Dir::Up;
            case 66:
                return Dir::Down;
            case 67:
                return Dir::Right;
            case 68:
                return Dir::Left;
            default:
                std::cin.ignore(3);
                break;
            }
        }

        return dir;
    }
    return Dir::None;
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
    auto teleport = screen_size - Coord{1, 1};

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

Dir dir = Dir::Right;
char ch = 'l';
void read_character() {
    while (ch != 'q' && ch != 'Q' && ch != 3 /* C-c */ && ch != 4 /* C-d */ && ch != 26 /* C-z */ && dir != Dir::None) {
        std::cin.get(ch);
        // get which direction the snake shall move to, if character is invalid, don't change: use `dir`
        auto prev_dir = dir;

        dir = from_char(ch, dir);
        // don't try to break your neck if you may!
        if (prev_dir == opposite(dir)) {
            dir = prev_dir;
        }
    }
    dir = Dir::None;
}

unsigned run(const Coord& screen_size) {
    auto apple = Coord::random(screen_size);
    apple.print(APPLE_TEXT);

    auto mid = screen_size / 2;
    Snake snake;
    for (auto i = 0; i < INIT_LEN; ++i) {
        snake.push_back(mid.with_col(mid.col - i));
    }
    auto score = Coord{2, 4};
    score.print(tui::string(tui::concat("score: ", snake.size() - INIT_LEN)).green().italic());

    while (dir != Dir::None) {
        // and move it correspondly
        move(snake, dir, screen_size);

        // die if wanna eat itself
        if (snake_contains(snake, snake.front(), 1)) {
            dir = Dir::None;
            return snake.size();
        }

        // snake ate apple, we need a new one!
        if (snake.front() == apple) {
            std::vector<Coord> non_snake;
            for (unsigned i = 1; i < screen_size.row; ++i) {
                for (unsigned j = 1; j < screen_size.col; ++j) {
                    if (!snake_contains(snake, Coord{i, j})) {
                        non_snake.emplace_back(i, j);
                    }
                }
            }
            if (non_snake.empty()) {
                dir = Dir::None;
                return snake.size();
            }
            std::mt19937 mt{std::random_device{}()};
            std::uniform_int_distribution<unsigned> gen_idx(0, non_snake.size() - 1);
            unsigned idx = gen_idx(mt);

            apple = non_snake.at(idx);
            // duplicate the last element of the `snake`, next round it'll be smoothed out.
            // assert(snake.size() + 1 == snake.previous_size())
            snake.push_back(snake.back());
            score.print(tui::string(tui::concat("score: ", snake.size() - INIT_LEN)).green().italic());
            apple.print(APPLE_TEXT);
        }

        // print non-head parts of snake, but only first 2
        for (auto i = 1; i < ((snake.size() == 1) ? 1 : 2); ++i) {
            auto nb = neighbours(snake, i, screen_size);
            snake[i].print(tui::string(draw(nb)).blue());
        }
        // print head
        snake.front().print(tui::string(to_string(dir)).blue());

        // it's kinda like a flush(): w/out this it's quite mad
        tui::cursor::set_position(screen_size.row - 1, screen_size.col - 1);
        std::cout << "\n";
        auto sleep_mul = (dir == Dir::Left || dir == Dir::Right) ? 1. : 1.5;
        auto sleep_dur = SLEEP_MS + (10 > snake.size() ? -(ADD_MS * 10) + ADD_MS * static_cast<unsigned>(snake.size())
                                                       : ADD_MS * static_cast<unsigned>(snake.size()));
        // sleep, if moving vertically: more
        std::this_thread::sleep_for(sleep_dur * sleep_mul);
    }
    return snake.size() - INIT_LEN;
}

int main() {
    try {
        tui::init(false);
        // we get screen size here, not to mess up cin, cout
        auto screen_size = Coord::screen_size();
        std::thread reader_thread(read_character);
        reader_thread.detach();

        auto len = run(screen_size);

        tui::reset();
        if (len == screen_size.row * screen_size.col) {
            std::cout << "Congrats, you won!\n";
        } else {
            std::cout << "You died/quit at " << len << "\nPress enter to quit if needed.\n";
        }
    } catch (...) {
        tui::reset();
        std::cerr << "unknown error occured\n";
    }

    return 0;
}
