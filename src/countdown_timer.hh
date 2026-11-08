#pragma once

#include <chrono>
#include <string>

class CountdownTimer {
public:
    using Clock = std::chrono::steady_clock;

    explicit CountdownTimer(std::chrono::seconds initial = std::chrono::seconds{0});

    void set_initial(std::chrono::seconds initial);
    std::chrono::seconds initial() const;
    std::chrono::seconds remaining() const;

    void start();
    void start(Clock::time_point now);
    void pause();
    void pause(Clock::time_point now);
    void reset();

    void update();
    void update(Clock::time_point now);

    bool running() const;
    bool started() const;
    bool expired() const;

    std::string remaining_text() const;

    static std::string format(std::chrono::seconds value);

private:
    std::chrono::seconds initial_;
    std::chrono::seconds remaining_;
    Clock::time_point deadline_;
    bool running_ = false;
    bool started_ = false;
    bool expired_ = false;
};
