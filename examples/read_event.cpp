#include "../input.hpp"
#include "../tui.hpp"

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;
std::condition_variable cv;
Input shared_input;
bool input_available = false;

bool should_quit(const Input& input) { return input == SpecKey::CtrlC || input == 'q'; }

void read_input() {
    while (!should_quit(shared_input)) {
        char c = 0;
        std::cin.get(c);

        std::lock_guard<std::mutex> lock(mtx);
        shared_input.read(c);
        input_available = true;
        cv.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
}

void main_task() {
    int i = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (input_available) {
                std::cout << "Character read from input: " << shared_input << "\r\n";
                if (should_quit(shared_input)) {
                    return;
                }
                input_available = false;
            }
        }

        std::cout << i << "\r\n";
        ++i;
    }
}

int main() {
    tui::init_term(false);
    std::thread reader_thread(read_input);

    main_task(); // Main thread doing its job

    reader_thread.join();

    tui::reset_term();
    return 0;
}
