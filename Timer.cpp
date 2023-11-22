#include <iostream>
#include <thread>
#include <chrono>

class Timer {
public:
    Timer() : seconds(0), running(false) {}

    Timer(int seconds) : seconds(seconds), running(false) {}

    void start() {
        running = true;
        start_time = std::chrono::high_resolution_clock::now();
        if (seconds > 0) {
            countdown_thread = std::thread(&Timer::countdown, this);
        }
    }

    void stop() {
        running = false;
        if (countdown_thread.joinable()) {
            countdown_thread.join();
        }
    }

    bool isRunning() {
        return running;
    }

    double get_elapsed_time() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double>(current_time - start_time).count();
    }

private:
    int seconds;
    bool running;
    std::thread countdown_thread;
    std::chrono::high_resolution_clock::time_point start_time;

    void countdown() {
        auto end_time = start_time + std::chrono::seconds(seconds);

        while (true) {
            if (std::chrono::high_resolution_clock::now() >= end_time) {
                std::cout << "Timer finished!" << std::endl;
                break;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};
