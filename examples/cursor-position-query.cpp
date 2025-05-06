#include "../tui.hpp"
#include <cassert>
#include <chrono>
#include <cstdint>
// #include <thread>

// returns how much the queries altogether took in ns
uint64_t get_cursor_pos_n(const unsigned n) {
    auto start = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < n; ++i) {
        // std::cout << "getting cursor position...\n";
        auto cursor_pos = tui::cursor::get_position();
        // std::cout << "cursor @ {" << cursor_pos.first << ", " << cursor_pos.second << "}\n";
        // assert(cursor_pos == cursor_pos_tty);
        // std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

int main(int argc, char** argv) {
    tui::enable_raw_mode();

    unsigned n = 8;
    if (argc > 1) {
        n = std::stoi(argv[1]);
    }

    auto query_t_sum = get_cursor_pos_n(n);
    // std::this_thread::sleep_for(std::chrono::seconds(8));
    tui::disable_raw_mode();
    std::cout << "getting cursor " << n << " times took: " << query_t_sum / 1000000 << "ms, avg: " << query_t_sum / n <<"ns\n";
    return 0;
}
