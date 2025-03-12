#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

class Timer {
public:
    Timer() : start_time_point(std::chrono::high_resolution_clock::now()), stopped(false), elapsed_time(0.0) {}

    void reset() {
        start_time_point = std::chrono::high_resolution_clock::now();
        stopped = false;
        elapsed_time = 0.0;
    }

    void stop() {
        if (!stopped) {
            auto end_time_point = std::chrono::high_resolution_clock::now();
            elapsed_time = std::chrono::duration<double>(end_time_point - start_time_point).count();
            stopped = true;
        }
    }

    double elapsed() const {
        if (stopped) {
            return elapsed_time;
        } else {
            return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start_time_point).count();
        }
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_point;
    bool stopped;
    double elapsed_time;
};

#endif // TIMER_HPP