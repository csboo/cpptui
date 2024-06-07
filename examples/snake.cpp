#include "../tui.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
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

    Dir meets_at(const Coord& other) const {
        int row_diff = this->row - other.row;
        int col_diff = this->col - other.col;

        if (row_diff == 1) {
            return Dir::Up;
        }
        if (row_diff == -1) {
            return Dir::Down;
        }
        if (col_diff == 1) {
            return Dir::Left;
        }
        if (col_diff == -1) {
            return Dir::Right;
        }
        return Dir::None;
    }
};
using Snake = std::vector<Coord>;

template <typename T> void print_at(const Coord& coord, const T& print) {
    tui::cursor::set_position(coord.row, coord.col);
    std::cout << print;
}

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

std::pair<Dir, Dir> neighbours(const Snake& snake, const unsigned& idx) {
    std::ofstream fout("babkelme.log", std::ios::app);
    auto coord = snake[idx];

    Coord prev{};
    if (!snake.empty()) {
        prev = snake[idx - 1];
    }
    Coord next{};
    if (idx < snake.size() - 1) {
        next = snake[idx + 1];
    }
    fout << "prev: " << prev.display() << "\ncoord: " << coord.display() << "\nnext: " << next.display() << "\n";

    Dir first = coord.meets_at(prev);
    fout << "prev is to the " << to_string(first) << "\n";
    Dir second = coord.meets_at(next);
    fout << "next is to the " << to_string(second) << "\n";

    fout.close();
    return {first, second};
}

std::string draw(const std::pair<Dir, Dir>& nb) {
    // rounded:  {"╭", "╮", "╰", "╯", "│", "─"}

    // this is where in Rust we'd use match and be happy
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

void handle_movement(const Dir& dir, Coord* coord, const Coord& ss) {
    switch (dir) {
    case Dir::Up:
        // move to the `Down` side of the screen if would go too far `Up`
        if (coord->row - 1 == 0) {
            coord->row = ss.row;
            break;
        }
        coord->row--;
        break;
    case Dir::Down:
        // move to the `Up`per side the screen if would go too far `Down`
        if (coord->row + 1 > ss.row) {
            coord->row = 1;
            break;
        }
        coord->row++;
        break;
    case Dir::Left:
        // move to the `Right` side the screen if would go too far `Left`
        if (coord->col - 1 == 0) {
            coord->col = ss.col;
            break;
        }
        coord->col--;
        break;
    case Dir::Right:
        // move to the `Left` side the screen if would go too far `Right`
        if (coord->col + 1 > ss.col) {
            coord->col = 1;
            break;
        }
        coord->col++;
        break;
    case Dir::None:
        break;
    }
}
void move(Snake& snake, const Dir& dir, const Coord& ss) {
    Coord* tail = &snake.back();
    Coord* head = &snake.front();

    // delete the last one off the screen by overwriting it with a space
    print_at(*tail, ' ');
    auto old_snake = snake;
    for (auto i = 1; i < snake.size(); ++i) {
        snake.at(i) = old_snake.at(i - 1);
    }

    handle_movement(dir, head, ss);
}

void new_tail(Snake& snake, const Coord& ss, const Dir& dir = Dir::Right) {
    if (snake.size() == 1) {
        auto snake_clone = snake;
        handle_movement(opposite(dir), &snake_clone.front(), ss);
        snake.push_back(snake_clone.front());
        return;
    }
    auto last = snake.at(snake.size() - 1);
    auto before_last = snake.at(snake.size() - 2);
    auto pos = before_last - last;
    snake.push_back(pos);
}

unsigned run() {
    auto screen_size = Coord::screen_size();

    auto apple_text = tui::tui_string('@').red().bold();
    auto apple = Coord::random(screen_size);
    print_at(apple, apple_text);

    char ch = 'l';
    auto dir = Dir::Right;

    Snake snake = {Coord{1, 0}};

    while (ch != 'q' && ch != 'Q' && ch != 3 /* C-c */ && ch != 4 /* C-d */ && ch != 26 /* C-z */) {
        // get which direction the snake shall move to, if character is invalid, don't change: use `dir`
        dir = from_char(ch, dir);

        // // automata
        // if (i % screen_size.row == 0) {
        //     dir = Dir::Right;
        //     saba = !saba;
        // } else {
        //     if (saba) {
        //         dir = Dir::Down;
        //     } else {
        //         dir = Dir::Up;
        //     }
        // }

        // and move it correspondly
        move(snake, dir, screen_size);

        // die if on itself
        auto x =
            std::find_if(std::begin(snake) + 1, snake.end(), [&](const Coord& item) { return item == snake.front(); });
        if (x != snake.end()) {
            return snake.size();
        }

        // snake ate apple, we need a new one!
        if (snake.front() == apple) {
            new_tail(snake, screen_size, dir);

            apple = Coord::random(screen_size);
        }

        for (auto i = 1; i < snake.size(); ++i) {
            auto item = snake[i];
            tui::cursor::set_position(item.row, item.col);
            auto y = neighbours(snake, i);
            auto x = draw(y);
            print_at(item, x);
        }
        print_at(snake.front(), to_string(dir));
        print_at(apple, apple_text);

        // TODO: other thread with mutex and stuff
        std::cin.get(ch);
        // LOGF << "\'" << ch << "\'\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    return 0;
}

int main() {
    tui::init_term(false);

    auto len = run();

    tui::reset_term();
    std::cout << "You died at " << len << std::endl;
    return 0;
}
