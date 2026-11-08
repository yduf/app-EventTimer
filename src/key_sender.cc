#include "key_sender.hh"

#include "x11_window.hh"

#include <X11/Xlib.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#if defined(__unix__) || defined(__APPLE__)
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

#if defined(__unix__) || defined(__APPLE__)

bool timer_is_focused() {
    FILE *pipe = popen("xdotool getwindowfocus getwindowpid 2>/dev/null", "r");
    if (pipe == nullptr) {
        return false;
    }

    char buffer[64] = {};
    const bool read = std::fgets(buffer, sizeof(buffer), pipe) != nullptr;
    pclose(pipe);

    if (!read) {
        return false;
    }

    char *end = nullptr;
    const long pid = std::strtol(buffer, &end, 10);
    return pid > 0 && static_cast<long>(getpid()) == pid;
}

bool target_is_self(unsigned long window) {
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        return false;
    }

    const long pid = x11::window_pid(display, window);
    XCloseDisplay(display);
    return pid > 0 && static_cast<long>(getpid()) == pid;
}

std::optional<unsigned long> resolve_target(const TargetWindow &target) {
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        return std::nullopt;
    }

    const auto window = x11::find_window(display, target);
    XCloseDisplay(display);
    return window;
}

enum class SpawnResult {
    Ok,
    Missing,
    Failed,
};

SendResult to_send_result(SpawnResult result) {
    switch (result) {
    case SpawnResult::Ok:
        return SendResult::Sent;
    case SpawnResult::Missing:
        return SendResult::Unavailable;
    case SpawnResult::Failed:
        return SendResult::Failed;
    }
    return SendResult::Failed;
}

SpawnResult spawn_xdotool(const std::vector<std::string> &arguments) {
    char program[] = "xdotool";
    std::vector<char *> argv;
    argv.reserve(arguments.size() + 2);
    argv.push_back(program);
    for (const auto &argument : arguments) {
        argv.push_back(const_cast<char *>(argument.c_str()));
    }
    argv.push_back(nullptr);

    pid_t pid = 0;
    const int spawn_error = posix_spawnp(&pid, "xdotool", nullptr, nullptr, argv.data(), environ);
    if (spawn_error != 0) {
        return SpawnResult::Missing;
    }

    int status = 0;
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            return SpawnResult::Failed;
        }
    }

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return SpawnResult::Failed;
    }

    return SpawnResult::Ok;
}

#endif

}

SendResult KeySender::send(const EventKey &key,
                           const std::optional<TargetWindow> &target) const {
    if (key.empty()) {
        return SendResult::NoEvent;
    }

#if defined(__unix__) || defined(__APPLE__)
    const auto sequence = key.xdotool_sequence();
    if (sequence.empty()) {
        return SendResult::Failed;
    }

    if (target && !target->empty()) {
        const auto window = resolve_target(*target);
        if (!window) {
            return SendResult::TargetNotFound;
        }

        if (target_is_self(*window)) {
            return SendResult::SelfFocused;
        }

        return to_send_result(spawn_xdotool({"key", "--window", std::to_string(*window),
                                             "--clearmodifiers", sequence}));
    }

    if (timer_is_focused()) {
        return SendResult::SelfFocused;
    }

    return to_send_result(spawn_xdotool({"key", "--clearmodifiers", sequence}));
#endif

    return SendResult::Unavailable;
}

std::string_view describe(SendResult result) {
    switch (result) {
    case SendResult::Sent:
        return "Event sent";
    case SendResult::NoEvent:
        return "No event recorded";
    case SendResult::SelfFocused:
        return "Event not sent (Event Timer is focused)";
    case SendResult::TargetNotFound:
        return "Event not sent (target window not found)";
    case SendResult::Unavailable:
        return "Event not sent (xdotool is not available)";
    case SendResult::Failed:
        return "Event could not be sent";
    }
    return "Event could not be sent";
}
