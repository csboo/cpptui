#include "../tui.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

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
};
using Snake = std::vector<Coord>;

template <typename T> void print_at(const Coord& coord, const T& print) {
    tui::cursor::set_position(coord.row, coord.col);
    std::cout << print;
}

enum Direction {
    Up = 0,
    Down,
    Left,
    Right,
    None,
};
Direction from_char(const char& ch, const Direction& dir = Direction::Right) {
    switch (ch) {
    case 'k':
        return Direction::Up;
    case 'j':
        return Direction::Down;
    case 'l':
        return Direction::Right;
    case 'h':
        return Direction::Left;
    default:
        if (ch < 0) {
            std::cin.ignore();
        } else if (ch == 27 && std::cin.peek() == 91) {
            std::cin.ignore();
            auto sus = std::cin.get();
            switch (sus) {
            case 65:
                return Direction::Up;
            case 66:
                return Direction::Down;
            case 67:
                return Direction::Right;
            case 68:
                return Direction::Left;
            default:
                std::cin.ignore(3);
                break;
            }
        }

        return dir;
    }
    return Direction::None;
}
Direction opposite(const Direction& dir) {
    switch (dir) {
    case Direction::Up:
        return Direction::Down;
    case Direction::Down:
        return Direction::Up;
    case Direction::Left:
        return Direction::Right;
    case Direction::Right:
        return Direction::Left;
    case Direction::None:
        break;
    }
    return Direction::None;
}
std::string to_string(const Direction& dir) {
    switch (dir) {
    case Direction::Up:
        return "A";
    case Direction::Down:
        return "V";
    case Direction::Left:
        return "<";
    case Direction::Right:
        return ">";
    case Direction::None:
        break;
    }
    return "X";
}

void handle_movement(const Direction& dir, Coord* coord, const Coord& ss) {
    switch (dir) {
    case Direction::Up:
        // move to the `Down` side of the screen if would go too far `Up`
        if (coord->row - 1 == 0) {
            coord->row = ss.row;
            break;
        }
        coord->row--;
        break;
    case Direction::Down:
        // move to the `Up`per side the screen if would go too far `Down`
        if (coord->row + 1 > ss.row) {
            coord->row = 1;
            break;
        }
        coord->row++;
        break;
    case Direction::Left:
        // move to the `Right` side the screen if would go too far `Left`
        if (coord->col - 1 == 0) {
            coord->col = ss.col;
            break;
        }
        coord->col--;
        break;
    case Direction::Right:
        // move to the `Left` side the screen if would go too far `Right`
        if (coord->col + 1 > ss.col) {
            coord->col = 1;
            break;
        }
        coord->col++;
        break;
    case Direction::None:
        break;
    }
}
void move(Snake& snake, const Direction& dir, const Coord& ss) {
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

void new_tail(Snake& snake, const Coord& ss, const Direction& dir = Direction::Right) {
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
    auto dir = Direction::Right;

    Snake snake = {Coord{1, 0}};

    while (ch != 'q' && ch != 'Q' && ch != 3 /* C-c */ && ch != 4 /* C-d */ && ch != 26 /* C-z */) {
        // get which direction the snake shall move to, if character is invalid, don't change: use `dir`
        dir = from_char(ch, dir);
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

        std::for_each(std::begin(snake) + 1, snake.end(), [&](const Coord& item) {
            tui::cursor::set_position(item.row, item.col);
            print_at(item, '#');
        });
        print_at(snake.front(), tui::tui_string(to_string(dir)).cyan().bold());
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
