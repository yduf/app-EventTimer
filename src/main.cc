#include "app.h"

#include "countdown_timer.hh"

#include <chrono>
#include <string>

namespace {

std::chrono::seconds configured_seconds(const AppWindow &window) {
    return std::chrono::seconds(window.get_hours() * 3600 + window.get_minutes() * 60 +
                                window.get_seconds());
}

std::string status_text(const CountdownTimer &timer) {
    if (timer.expired()) {
        return "Finished";
    }
    if (timer.running()) {
        return "Running";
    }
    if (timer.started()) {
        return "Paused";
    }
    return "Ready";
}

}

int main() {
    auto app = AppWindow::create();
    CountdownTimer timer(configured_seconds(*app));
    slint::Timer tick;

    auto refresh = [&]() {
        app->set_remaining_text(slint::SharedString(timer.remaining_text()));
        app->set_status_text(slint::SharedString(status_text(timer)));
        app->set_running(timer.running());
        app->set_started(timer.started());
        app->set_can_start(timer.remaining() > std::chrono::seconds{0});
        app->set_expired(timer.expired());
    };

    auto on_tick = [&]() {
        timer.update();
        refresh();
        if (!timer.running()) {
            tick.stop();
        }
    };

    auto sync = [&]() {
        refresh();

        if (timer.running() && !tick.running()) {
            tick.start(slint::TimerMode::Repeated, std::chrono::milliseconds(100), on_tick);
        } else if (!timer.running() && tick.running()) {
            tick.stop();
        }
    };

    app->on_duration_changed([&](int hours, int minutes, int seconds) {
        timer.set_initial(std::chrono::seconds(hours * 3600 + minutes * 60 + seconds));
        sync();
    });

    app->on_start_pause([&]() {
        if (timer.running()) {
            timer.pause();
        } else {
            timer.start();
        }
        sync();
    });

    app->on_reset([&]() {
        timer.reset();
        sync();
    });

    app->on_dismiss_expired([&]() {
        timer.reset();
        sync();
    });

    sync();
    app->run();
    return 0;
}
