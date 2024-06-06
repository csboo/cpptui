#include "../tui.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

std::ofstream logfile("log.log");

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
        break;
    }
    // unreachable
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
    default:
        break;
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

using Snake = std::vector<Coord>;

void handle_movement(const Direction& dir, Coord*& coord) {
    switch (dir) {
    case Up:
        coord->row--;
        break;
    case Down:
        coord->row++;
        break;
    case Left:
        coord->col--;
        break;
    case Right:
        coord->col++;
        break;
    default:
        break;
    }
}
void move(Snake& snake, const Direction& dir) {
    Coord* tail = &snake.back();
    Coord* head = &snake.front();

    // delete the last one off the screen by overwriting it with a space
    tui::cursor::set_position(tail->row, tail->col);
    std::cout.put(' ');
    // std::cout << "tail change!";
    // snake.pop_back();
    // snake.push_back(snake.back());
    // tail = &snake.at(snake.size() - 2);
    for (auto i = 1; i < snake.size(); ++i) {
        logfile << "previous: " << snake.at(i - 1).display() << "\n";
        logfile << "current: " << snake.at(i).display() << "\n";

        snake.at(i) = snake.at(i - 1);
    }
    logfile << "\n";

    handle_movement(dir, head);
}

void run() {
    auto screen_size_pair = tui::screen::size();
    auto screen_size = Coord{screen_size_pair.first, screen_size_pair.second};

    char ch = 'l';
    auto apple = Coord::random();
    auto dir = Direction::Right;
    Snake snake = {Coord{1, 1}};

    while (ch != 'q') {
        dir = from_char(ch);
        move(snake, dir);
        // snake ate apple, we need a new one!
        if (snake.front() == apple) {
            snake.push_back(Coord{snake.back().row - 1, apple.col - 1});
            tui::cursor::set_position(30, 0);
            std::cout << "--apple--";
            logfile << "\nAPPLE\n";
            apple = Coord::random();
            tui::cursor::set_position(apple.row, apple.col);
            std::cout << tui::tui_string('@').red().bold();
        } else {
            tui::cursor::set_position(30, 0);
            std::cout << "--     --\r\n";
            auto i = 0;
            for (auto item : snake) {
                std::cout << i << ": " << item.display() << "           \r\n";
                i++;
            }
            std::cout << "\r\napple: " << apple.display() << "          \r\nlen: " << snake.size();
        }

        for (const Coord& item : snake) {
            tui::cursor::set_position(item.row, item.col);
            tui::tui_string x;
            if (item == snake.front()) {
                x += to_string(dir);
                x = x.cyan();
            } else if (item == snake.back()) {
                x = "&";
            } else {
                x = "-";
            }
            std::cout << x;
        }

        std::cin.get(ch);
        logfile << "\'" << ch << "\'\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

int main() {
    tui::init_term(false);

    run();

    tui::reset_term();
    return 0;
}
