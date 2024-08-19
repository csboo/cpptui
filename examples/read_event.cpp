#include "../tui.hpp"

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;
std::condition_variable cv;
char shared_char;
bool char_available = false;

void read_character() {
    while (shared_char != 'q') {
        char c = 0;
        std::cin >> c;

        std::lock_guard<std::mutex> lock(mtx);
        shared_char = c;
        char_available = true;
        cv.notify_one();
    }
}

void main_task() {
    int i = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (char_available) {
                std::cout << "Character read from input: " << shared_char << "\r\n";
                if (shared_char == 'q') {
                    return;
                }
                char_available = false;
            }
        }

        std::cout << i << "\r\n";
        ++i;
    }
}

int main() {
    std::thread reader_thread(read_character);
    tui::init_term(false);

    main_task(); // Main thread doing its job

    reader_thread.join();

    tui::reset_term();
    return 0;
}
