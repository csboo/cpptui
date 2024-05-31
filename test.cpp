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
            this->content.resize((width+1)*(height+1));
            std::cerr << content.size()<< "resize done\n";
            // for (auto & i : content) {
            //     i = "X";
            // }
            init_box();
        }

        // void add_text(const std::string& text, Align alignment = Align::TopLeft, int x = 0, int y = 0) {
            // texts.push_back({text, alignment, x, y});
        // }

        void init_box() {
            const Box_chars& chars = styles.at(style);

            // Top border
            content.at(getij(0, 0)) = chars.corners[0]; 
            std::cerr << "topleft done\n";
            // std::cerr << getij(0,0) << "\n";

            for(int j = 1; j < width; j++){
               content.at(getij(0, j)) = chars.horizontal;
            }
            std::cerr << "topdone\n";

            content.at(getij(0, width)) = chars.corners[1];
            std::cerr << "topright done\n";
            
            // Side borders
            for (int i = 1; i < height; i++) {
                content.at(getij(i, 0)) = chars.vertical;
            }
            for (int i = 1; i < height; i++) {
                content.at(getij(i, width)) = chars.vertical;
            }
            std::cerr << "side done\n";
            // Bottom border
            content.at(getij(height, 0)) = chars.corners[3];
            std::cerr << "bottom left done\n";
            for (int j = 1; j < width; j++ ) {
                content.at(getij(height, j)) = chars.horizontal; 
            }
            std::cerr << "bottom done\n";
            content.at(getij(height, width)) = chars.corners[2];
            std::cerr << "last done\n";
        };
        void print(){
            // for (int i = 0; i < width; i++) {
            //     for (int j = 0; j < height; j++) {
            //         std::cout << getij(i, j);
            //     }
            //     std::cout << "\n";
            // }
            int c =0;
            for (int i = 0; i < height+1; i++) {
                for (int j = 0; j < width+1; j++ ) {
                    if (j==0) {
                        if (c==10) {
                            std::cout << 1;
                            c++;
                        } else {
                            std::cout << c++;
                        }
                        if (c==11){
                            c = 1;
                        }
                    }
                    std::cout << content.at(getij(i,j));
                }
                std::cout << "\n";
            }
        }

        inline int get_width() const {return this->width;}

        inline int getij(int i, int j) const {
            return j + i * get_width();
        };
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

        const int width;
        const int height;

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
        
        std::vector<std::string> content;
        

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
    int width = 20;
    int height = 10;
    tui::Box alma(width, height, tui::Box::Style::Rounded);
    std::cerr << "main box init done\n";
    alma.print();
    return 0;
}
