#include "../input.hpp"
#include "../tui.hpp"
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

static std::mutex mtx;
static std::condition_variable cv;
static Input shared_input;
static bool input_available = false;

bool should_quit(const Input& input) { return input == SpecKey::CtrlC || input == 'q'; }

void read_input() {
    while (!should_quit(shared_input)) {
        shared_input = Input::read();
        std::lock_guard<std::mutex> lock(mtx);
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
    tui::init(false);
    std::thread reader_thread(read_input);

    main_task(); // Main thread doing its job

    reader_thread.join();

    tui::reset();
    return 0;
}
