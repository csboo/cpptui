#include "../tui.hpp"
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
    static Coord random() {
        auto ss = tui::screen::size();
        std::mt19937 mt{std::random_device{}()};
        std::uniform_int_distribution<unsigned> gen_y(1, ss.first);
        unsigned y = gen_y(mt);
        std::uniform_int_distribution<unsigned> gen_x(1, ss.second);
        unsigned x = gen_x(mt);

        return Coord{y, x};
    }
    std::pair<unsigned, unsigned> to_pair() const { return {this->row, this->col}; }
    std::string display() const { return "(" + std::to_string(this->row) + ";" + std::to_string(this->col) + ")"; }
};

enum Direction {
    Up = 0,
    Down,
    Left,
    Right,
};

using Snake = std::vector<Coord>;

void move(Snake& snake, const Direction& dir) {
    // delete the last one
    auto last = snake.back();
    tui::cursor::set_position(last.row, last.col);
    std::cout.put(' ');

    auto* head = &snake.front();

    switch (dir) {
    case Up:
        head->row--;
        break;
    case Down:
        head->row++;
        break;
    case Left:
        head->col--;
        break;
    case Right:
        head->col++;
        break;
    default:
        break;
    }
}
Direction from_char(const char& ch) {
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
        // unreachable
        ;
    }
    // unreachable
}

void run() {
    auto screen_size_pair = tui::screen::size();
    auto screen_size = Coord{screen_size_pair.first, screen_size_pair.second};

    char ch = 'l';
    auto apple = Coord{1, 2};
    auto dir = Direction::Right;
    Snake snake = {Coord{1, 1}};

    while (ch != 'q') {
        dir = from_char(ch);
        move(snake, dir);
        // snake ate apple, we need a new one!
        if (snake.front() == apple) {
            snake.push_back(apple);
            tui::cursor::set_position(30, 0);
            std::cout << "--apple--";
            apple = Coord::random();
            tui::cursor::set_position(apple.row, apple.col);
            std::cout.put('x');
        } else {
            tui::cursor::set_position(30, 0);
            std::cout << "--     --\r\n"
                      << snake.front().display() << "\r\n"
                      << snake.back().display() << "\r\n"
                      << apple.display();
        }

        for (const Coord& item : snake) {
            tui::cursor::set_position(item.row, item.col);
            std::cout.put((item == snake.front() ? '@' : '#'));
        }

        std::cin.get(ch);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

int main() {
    tui::init_term(false);

    run();

    tui::reset_term();
    return 0;
}
