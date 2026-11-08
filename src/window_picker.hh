#pragma once

#include "target_window.hh"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

struct WindowInfo {
    unsigned long id_ = 0;
    std::string class_name_;
    std::string title_;
    bool self_ = false;

    bool empty() const;
    TargetWindow target() const;
};

enum class PickState {
    Idle,
    Pending,
    Selected,
    Self,
    Failed,
};

class WindowPicker {
public:
    WindowPicker() = default;
    ~WindowPicker();

    WindowPicker(const WindowPicker &) = delete;
    WindowPicker &operator=(const WindowPicker &) = delete;

    PickState start();
    PickState poll(WindowInfo &info);
    bool pending() const;

private:
    void pick_thread();

    std::atomic<PickState> state_{PickState::Idle};
    std::atomic<bool> stop_{false};
    std::mutex mutex_;
    WindowInfo info_;
    std::thread thread_;
};
