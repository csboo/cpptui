#include "tui.hpp"
#include <iostream>
#include <math.h>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace tui::text::style;
using namespace tui::text::color;

namespace tui {

    class Box {
      public:
        enum class Style { Empty, Light, Heavy, Double, Dashed, Rounded };
        enum class Align { TopLeft, TopRight, BottomLeft, BottomRight, Center, Custom };
        Box(int width, int height, Style style = Style::Empty, std::string title = "")
            : width(width), height(height), title(std::move(title)), style(style) {
            init_box();
        }

        // void add_text(const std::string& text, Align alignment = Align::TopLeft, int x = 0, int y = 0) {
            // texts.push_back({text, alignment, x, y});
        // }

        void init_box() {
            const Box_chars& chars = styles.at(style);

            // Top border
            
            // Side borders

            // Bottom border
        };
        void print(){
            
        }

        inline int get_width() const {return this->width;}
        inline void set_width(unsigned width) {this->width = width;}
        inline int get_height() const {return this->width;}
        inline void set_height(unsigned height) {this->height = height;}

      private:
        struct Box_chars {
            std::string corners[4];
            std::string horizontal;
            std::string vertical;
        };

        struct Text {
            std::string content;
            Align alignment;
            int x;
            int y;
        };

        struct Coord{
            int x;
            int y;
        };

        int width;
        int height;

        Coord astart;
        Coord aend;
        Coord rstart;
        Coord rend;
        
        std::string title;

        Style style;
        std::vector<Text> texts;
        const std::unordered_map<Style, Box_chars> styles = {
            {Style::Light, {{"┌", "┐", "┘", "└"}, "─", "│"}},
            {Style::Heavy,  {{"┏", "┓", "┛", "┗"}, "━", "┃"}},
            {Style::Double, {{"╔", "╗", "╝", "╚"}, "═", "║"}},
            {Style::Dashed, {{"┌", "┐", "┘", "└"}, "┄", "┆"}},
            {Style::Empty, {{" ", " ", " ", " "}, " ", " "}},
            {Style::Rounded, {{"╭", "╮", "╯", "╰"}, "─", "│"}},
        };

        // int get_align(Align alignment) const {
        //     switch (alignment) {
        //     case Align::TopLeft:
        //     case Align::TopRight:
        //         return 0;
        //     case Align::BottomLeft:
        //     case Align::BottomRight:
        //         return height - 3;
        //     case Align::Center:
        //         return (height - 2) / 2;
        //     default:
        //         return 0;
        //     }
        // }

        // std::string get_text(int line, const Box_chars& chars) const {
        //     const Text& text = texts[0]; // Simplification: handle only the first text for now
        //     int padding = (width - 2 - text.content.size()) / 2;
        //     switch (text.alignment) {
        //     case Align::TopLeft:
        //     case Align::BottomLeft:
        //         return text.content + std::string(width - 2 - text.content.size(), ' ');
        //     case Align::TopRight:
        //     case Align::BottomRight:
        //         return std::string(width - 2 - text.content.size(), ' ') + text.content;
        //     case Align::Center:
        //         return std::string(padding, ' ') + text.content +
        //                std::string(width - 2 - padding - text.content.size(), ' ');
        //     default:
        //         return text.content;
        //     }
        // }
    };
} // namespace tui

int main() {
    tui::init_term(true);
    
    int width = 20;
    int height = 10;
    tui::Box alma(width, height, tui::Box::Style::Rounded);
    std::cerr << "main box init done\n";
    alma.print();
    
    char car = '\0';
    // tui::cursor::set_position();
    std::cout << "q to quit";
    while(car != 'q'){ std::cin.get(car); }
    tui::reset_term();
    return 0;
}
