#pragma once

#include "tui.hpp"
#include <random>
#include <utility>

struct Coord {
    // lines, y axis
    unsigned row = 0;
    // columns, x axis
    unsigned col = 0;

    // constructors
    Coord() = default;
    Coord(const unsigned& row, const unsigned& col) : row{row}, col{col} {}
    Coord(const std::pair<unsigned, unsigned>& copy) : row{copy.first}, col{copy.second} {}

    Coord with_col(const unsigned& col) const { return Coord{this->row, col}; }
    Coord with_row(const unsigned& row) const { return Coord{row, this->col}; }

    // operator overloads
    bool operator!=(const Coord& other) const { return !(this == &other); }
    bool operator==(const Coord& other) const { return (this->row == other.row && this->col == other.col); }
    bool operator<=(const Coord& other) const { return (this->row <= other.row && this->col <= other.col); }
    Coord operator/(const unsigned& n) const { return Coord{this->row / n, this->col / n}; }
    // See what I did there?
    // we subtract the difference of the two coordinates
    // Coord operator-(const Coord& other) const {
    //     return Coord{
    //         this->row - static_cast<int>(static_cast<int>(this->row) - static_cast<int>(other.row)),
    //         this->col - static_cast<int>(static_cast<int>(this->col) - static_cast<int>(other.col)),
    //     };
    // }
    // Coord operator=(std::pair<unsigned, unsigned> other) {
    //     this->row = other.first;
    //     this->col = other.second;
    //     return *this;
    // }

    // convert to a pair `{row, col}`
    std::pair<unsigned, unsigned> to_pair() const { return {this->row, this->col}; }

    // get terminal screen size
    static Coord screen_size() {
        auto ss = tui::screen::size();
        return Coord{ss.first, ss.second};
    }

    // generate a random `Coord` in bounds of `screen_size`
    static Coord random(const Coord& screen_size) {
        std::mt19937 mt{std::random_device{}()};

        std::uniform_int_distribution<unsigned> gen_y(1, screen_size.row - 1);
        unsigned y = gen_y(mt);

        std::uniform_int_distribution<unsigned> gen_x(1, screen_size.col - 1);
        unsigned x = gen_x(mt);

        return Coord{y, x};
    }

    std::string display() const { return "(" + std::to_string(this->row) + ";" + std::to_string(this->col) + ")"; }

    // print to `stdout` starting from this `Coord`
    template <typename T> void print(const T& print) {
        tui::cursor::set_position(this->row, this->col);
        std::cout << print;
    }
};
