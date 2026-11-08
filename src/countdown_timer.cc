#include "countdown_timer.hh"

#include <format>

namespace {

constexpr std::chrono::seconds zero{0};

std::chrono::seconds non_negative(std::chrono::seconds value) {
    return value < zero ? zero : value;
}

std::chrono::seconds seconds_left(CountdownTimer::Clock::time_point deadline,
                                  CountdownTimer::Clock::time_point now) {
    if (deadline <= now) {
        return zero;
    }
    const auto difference = deadline - now;
    auto left = std::chrono::duration_cast<std::chrono::seconds>(difference);
    if (left < difference) {
        left += std::chrono::seconds{1};
    }
    return left;
}

}

CountdownTimer::CountdownTimer(std::chrono::seconds initial)
    : initial_(non_negative(initial)), remaining_(non_negative(initial)) {
}

void CountdownTimer::set_initial(std::chrono::seconds initial) {
    initial_ = non_negative(initial);
    reset();
}

std::chrono::seconds CountdownTimer::initial() const {
    return initial_;
}

std::chrono::seconds CountdownTimer::remaining() const {
    return remaining_;
}

void CountdownTimer::start() {
    start(Clock::now());
}

void CountdownTimer::start(Clock::time_point now) {
    if (running_ || remaining_ <= zero) {
        return;
    }
    deadline_ = now + remaining_;
    running_ = true;
    started_ = true;
}

void CountdownTimer::pause() {
    pause(Clock::now());
}

void CountdownTimer::pause(Clock::time_point now) {
    if (!running_) {
        return;
    }
    update(now);
    running_ = false;
}

void CountdownTimer::reset() {
    running_ = false;
    started_ = false;
    expired_ = false;
    remaining_ = initial_;
}

void CountdownTimer::update() {
    update(Clock::now());
}

void CountdownTimer::update(Clock::time_point now) {
    if (!running_) {
        return;
    }
    remaining_ = seconds_left(deadline_, now);
    if (remaining_ <= zero) {
        remaining_ = zero;
        running_ = false;
        expired_ = true;
    }
}

bool CountdownTimer::running() const {
    return running_;
}

bool CountdownTimer::started() const {
    return started_;
}

bool CountdownTimer::expired() const {
    return expired_;
}

std::string CountdownTimer::remaining_text() const {
    return format(remaining_);
}

std::string CountdownTimer::format(std::chrono::seconds value) {
    const auto total = non_negative(value).count();
    const auto hours = total / 3600;
    const auto minutes = (total % 3600) / 60;
    const auto seconds = total % 60;
    return std::format("{:02}:{:02}:{:02}", hours, minutes, seconds);
}
