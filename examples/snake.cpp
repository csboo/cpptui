#include "../tui.hpp"
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

std::ofstream LOGF("log.log");

struct Coord {
    unsigned row;
    unsigned col;

    bool operator==(const Coord& other) const { return (this->row == other.row && this->col == other.col); }
    // See what I did there?
    Coord operator-(const Coord& other) const {
        return Coord{
            this->row + static_cast<unsigned>(abs(static_cast<int>(this->row - static_cast<int>(other.row)))),
            this->col + static_cast<unsigned>(abs(static_cast<int>(this->col - static_cast<int>(other.col)))),
        };
    }
    static Coord screen_size() {
        auto ss = tui::screen::size();
        return Coord{ss.first, ss.second};
    }
    static Coord random() {
        auto ss = Coord::screen_size();
        std::mt19937 mt{std::random_device{}()};
        std::uniform_int_distribution<unsigned> gen_y(1, ss.row);
        unsigned y = gen_y(mt);
        std::uniform_int_distribution<unsigned> gen_x(1, ss.col);
        unsigned x = gen_x(mt);

        return Coord{y, x};
    }
    std::pair<unsigned, unsigned> to_pair() const { return {this->row, this->col}; }
    std::string display() const { return "(" + std::to_string(this->row) + ";" + std::to_string(this->col) + ")"; }
};
using Snake = std::vector<Coord>;

template <typename T> void print_at(const T& print, const Coord& coord) {
    tui::cursor::set_position(coord.row, coord.col);
    std::cout << print;
}

enum Direction {
    Up = 0,
    Down,
    Left,
    Right,
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
        return dir;
    }
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
    }
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
    }
}

void handle_movement(const Direction& dir, Coord* coord) {
    switch (dir) {
    case Direction::Up:
        coord->row--;
        break;
    case Direction::Down:
        coord->row++;
        break;
    case Direction::Left:
        coord->col--;
        break;
    case Direction::Right:
        coord->col++;
        break;
    }
}
void move(Snake& snake, const Direction& dir) {
    Coord* tail = &snake.back();
    Coord* head = &snake.front();

    // delete the last one off the screen by overwriting it with a space
    print_at(' ', *tail);
    auto old_snake = snake;
    for (auto i = 1; i < snake.size(); ++i) {
        snake.at(i) = old_snake.at(i - 1);
    }

    handle_movement(dir, head);
}

void new_tail(Snake& snake, const Direction& dir = Direction::Right) {
    if (snake.size() == 1) {
        auto snake_clone = snake;
        handle_movement(opposite(dir), &snake_clone.front());
        snake.push_back(snake_clone.front());
        return;
    }
    auto last = snake.at(snake.size() - 1);
    auto before_last = snake.at(snake.size() - 2);
    auto pos = before_last - last;
    snake.push_back(pos);
}

void run() {
    auto screen_size = Coord::screen_size();

    auto apple_text = tui::tui_string('@').red().bold();

    char ch = 'l';
    auto apple = Coord::random();
    print_at(apple_text, apple);
    auto dir = Direction::Right;
    Snake snake = {Coord{1, 1}};

    while (ch != 'q') {
        // get which direction the snake shall move to, if character is invalid, don't change: use `dir`
        dir = from_char(ch, dir);
        // and move it correspondly
        move(snake, dir);

        // snake ate apple, we need a new one!
        if (snake.front() == apple) {
            new_tail(snake, dir);

            apple = Coord::random();
            LOGF << apple.display() << "\n";
            print_at(apple_text, apple);
        }

        for (auto i = 0; i < snake.size(); ++i) {
            auto item = snake[i];
            tui::cursor::set_position(item.row, item.col);
            tui::tui_string x;
            if (item == snake.front()) {
                x += to_string(dir);
                x = x.cyan().bold();
            } else {
                x = "#";
            }
            std::cout << x;
        }

        // TODO: other thread with mutex and stuff
        std::cin.get(ch);
        // TODO: handle all kinds of strange chars: ő, ->, ...
        // LOGF << "\'" << ch << "\'\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

int main() {
    tui::init_term(false);

    run();

    LOGF.close();
    tui::reset_term();
    return 0;
}
