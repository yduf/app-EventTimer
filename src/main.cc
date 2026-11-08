#include "app.h"

#include "config.hh"
#include "countdown_timer.hh"
#include "event_key.hh"
#include "key_sender.hh"
#include "window_picker.hh"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

#if defined(__unix__)
#include <X11/Xlib.h>
#endif

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
#if defined(__unix__)
    XInitThreads();
#endif

    auto app = AppWindow::create();
    Config config = Config::load();
    std::optional<EventKey> event = config.event_;
    std::optional<TargetWindow> target = config.target_;

    if (event && !event->empty()) {
        app->set_event_key(slint::SharedString(event->display()));
    }

    if (target && !target->empty()) {
        app->set_target_name(slint::SharedString(target->display()));
    }

    CountdownTimer timer(configured_seconds(*app));
    KeySender sender;
    WindowPicker picker;
    slint::Timer tick;
    slint::Timer pick_timer;
    bool event_sent = false;

    auto send_event_if_needed = [&]() {
        if (!timer.expired() || event_sent) {
            return;
        }

        event_sent = true;

        if (!event || event->empty()) {
            app->set_send_status("No event recorded");
            return;
        }

        const auto result = sender.send(*event, target);
        app->set_send_status(slint::SharedString(std::string(describe(result))));
    };

    auto refresh = [&]() {
        app->set_remaining_text(slint::SharedString(timer.remaining_text()));
        app->set_status_text(slint::SharedString(status_text(timer)));
        app->set_running(timer.running());
        app->set_started(timer.started());
        app->set_can_start(timer.remaining() > std::chrono::seconds{0});
        app->set_expired(timer.expired());
        if (timer.expired()) {
            app->set_recording(false);
        }
    };

    auto on_tick = [&]() {
        timer.update();
        send_event_if_needed();
        refresh();
        if (!timer.running()) {
            tick.stop();
        }
    };

    auto sync = [&]() {
        send_event_if_needed();
        refresh();

        if (timer.running() && !tick.running()) {
            tick.start(slint::TimerMode::Repeated, std::chrono::milliseconds(100), on_tick);
        } else if (!timer.running() && tick.running()) {
            tick.stop();
        }
    };

    auto poll_pick = [&]() {
        WindowInfo info;
        const auto state = picker.poll(info);
        if (state == PickState::Pending) {
            return;
        }

        pick_timer.stop();
        app->set_picking(false);

        switch (state) {
        case PickState::Selected:
            target = info.target();
            app->set_target_name(slint::SharedString(target->display()));
            app->set_target_status("");
            config.target_ = target;
            config.save();
            break;
        case PickState::Self:
            app->set_target_status("Cannot target Event Timer");
            break;
        case PickState::Failed:
            app->set_target_status("Could not select a target window");
            break;
        case PickState::Idle:
            app->set_target_status("Target selection cancelled");
            break;
        case PickState::Pending:
            break;
        }
    };

    app->on_duration_changed([&](int hours, int minutes, int seconds) {
        timer.set_initial(std::chrono::seconds(hours * 3600 + minutes * 60 + seconds));
        event_sent = false;
        app->set_send_status("");
        sync();
    });

    app->on_start_pause([&]() {
        app->set_recording(false);
        app->set_target_status("");
        if (timer.running()) {
            timer.pause();
        } else {
            event_sent = false;
            app->set_send_status("");
            timer.start();
        }
        sync();
    });

    app->on_reset([&]() {
        app->set_recording(false);
        app->set_target_status("");
        timer.reset();
        event_sent = false;
        app->set_send_status("");
        sync();
    });

    app->on_dismiss_expired([&]() {
        app->set_recording(false);
        timer.reset();
        event_sent = false;
        app->set_send_status("");
        sync();
    });

    app->on_record_key([&](slint::SharedString text, bool alt, bool control, bool shift,
                           bool meta) {
        auto recorded = EventKey::from_slint_text(std::string_view(text), alt, control, shift, meta);
        if (!recorded) {
            app->set_recording(false);
            return;
        }

        event = std::move(recorded);
        app->set_event_key(slint::SharedString(event->display()));
        app->set_recording(false);

        config.event_ = event;
        config.save();
    });

    app->on_pick_target([&]() {
        if (picker.pending()) {
            return;
        }

        const auto state = picker.start();
        if (state == PickState::Pending) {
            app->set_picking(true);
            app->set_target_status("");
            pick_timer.start(slint::TimerMode::Repeated, std::chrono::milliseconds(100),
                             poll_pick);
        } else {
            app->set_target_status("Could not start window picker");
        }
    });

    sync();
    app->run();
    return 0;
}
