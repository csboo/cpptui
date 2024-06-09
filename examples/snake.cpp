#include "../tui.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

// direction
enum Dir {
    Up = 0,
    Down,
    Left,
    Right,
    None,
};

struct Coord {
    unsigned row;
    unsigned col;

    bool operator==(const Coord& other) const { return (this->row == other.row && this->col == other.col); }
    bool operator<=(const Coord& other) const { return (this->row <= other.row && this->col <= other.col); }
    // See what I did there?
    // we subtract the difference of the two coordinates
    Coord operator-(const Coord& other) const {
        return Coord{
            this->row - static_cast<int>(static_cast<int>(this->row) - static_cast<int>(other.row)),
            this->col - static_cast<int>(static_cast<int>(this->col) - static_cast<int>(other.col)),
        };
    }
    static Coord screen_size() {
        auto ss = tui::screen::size();
        return Coord{ss.first, ss.second};
    }
    static Coord random(const Coord& ss) {
        std::mt19937 mt{std::random_device{}()};

        std::uniform_int_distribution<unsigned> gen_y(1, ss.row - 1);
        unsigned y = gen_y(mt);

        std::uniform_int_distribution<unsigned> gen_x(1, ss.col - 1);
        unsigned x = gen_x(mt);

        return Coord{y, x};
    }
    std::pair<unsigned, unsigned> to_pair() const { return {this->row, this->col}; }
    std::string display() const { return "(" + std::to_string(this->row) + ";" + std::to_string(this->col) + ")"; }

    Dir meets_at(const Coord& other, const Coord& ss) const {
        int row_diff = static_cast<int>(this->row) - static_cast<int>(other.row);
        int col_diff = static_cast<int>(this->col) - static_cast<int>(other.col);

        // we set both row and col to x-1 as it's needed :D
        auto teleport = Coord{ss.row - 1, ss.col - 1};

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

    // print starting from this Coord
    template <typename T> void print(const T& print) {
        tui::cursor::set_position(this->row, this->col);
        std::cout << print;
    }
};
using Snake = std::vector<Coord>;

Dir from_char(const char& ch, const Dir& dir = Dir::Right) {
    switch (ch) {
    case 'k':
        return Dir::Up;
    case 'j':
        return Dir::Down;
    case 'l':
        return Dir::Right;
    case 'h':
        return Dir::Left;
    default:
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

std::pair<Dir, Dir> neighbours(const Snake& snake, const unsigned& idx, const Coord& ss) {
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

    Dir first = coord.meets_at(prev, ss);
    Dir second = coord.meets_at(next, ss);

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

unsigned run() {
    auto screen_size = Coord::screen_size();

    auto apple_text = tui::tui_string('@').red().bold();
    auto apple = Coord::random(screen_size);
    apple.print(apple_text);

    char ch = 'l';
    auto dir = Dir::Right;

    Snake snake = {Coord{1, 0}};

    while (ch != 'q' && ch != 'Q' && ch != 3 /* C-c */ && ch != 4 /* C-d */ && ch != 26 /* C-z */) {
        // get which direction the snake shall move to, if character is invalid, don't change: use `dir`
        dir = from_char(ch, dir);

        // and move it correspondly
        move(snake, dir, screen_size);

        // die if on itself
        if (snake_contains(snake, snake.front(), 1)) {
            return snake.size();
        }

        // snake ate apple, we need a new one!
        if (snake.front() == apple) {
            // generate, till it's not on the `snake` itself
            do {
                apple = Coord::random(screen_size);
            } while (snake_contains(snake, apple));
            apple.print(apple_text);
            // duplicate the last element of the `snake`, next round it'll be smoothed out.
            // assert(snake.size() + 1 == snake.previous_size())
            snake.push_back(snake.back());
        }

        for (auto i = 1; i < ((snake.size() == 1) ? 1 : 2); ++i) {
            auto nb = neighbours(snake, i, screen_size);
            snake[i].print(draw(nb));
        }
        snake.front().print(to_string(dir));

        // TODO: other thread with mutex and stuff
        std::cin.get(ch);

        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    return snake.size();
}

int main() {
    tui::init_term(false);

    auto len = run();

    tui::reset_term();
    std::cout << "You died/quit at " << len << std::endl;
    return 0;
}
