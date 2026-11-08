#include "window_picker.hh"

#include "x11_window.hh"

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <chrono>
#include <unistd.h>

namespace {

Window deepest_child(Display *display, Window window) {
    Window current = window;

    for (;;) {
        Window root_return = 0;
        Window child_return = 0;
        int root_x = 0;
        int root_y = 0;
        int win_x = 0;
        int win_y = 0;
        unsigned int mask = 0;

        if (!XQueryPointer(display, current, &root_return, &child_return, &root_x,
                           &root_y, &win_x, &win_y, &mask)) {
            break;
        }
        if (child_return == None) {
            break;
        }
        current = child_return;
    }

    return current;
}

}

bool WindowInfo::empty() const {
    return class_name_.empty() && title_.empty();
}

TargetWindow WindowInfo::target() const {
    return TargetWindow{class_name_, title_};
}

WindowPicker::~WindowPicker() {
    stop_.store(true);
    if (thread_.joinable()) {
        thread_.join();
    }
}

PickState WindowPicker::start() {
    if (state_.load() == PickState::Pending) {
        return PickState::Pending;
    }

    stop_.store(false);
    if (thread_.joinable()) {
        thread_.join();
    }

    state_.store(PickState::Pending);
    thread_ = std::thread(&WindowPicker::pick_thread, this);
    return PickState::Pending;
}

PickState WindowPicker::poll(WindowInfo &info) {
    const auto state = state_.load();
    if (state == PickState::Selected || state == PickState::Self) {
        std::lock_guard<std::mutex> lock(mutex_);
        info = info_;
        state_.store(PickState::Idle);
        return state;
    }

    if (state == PickState::Failed) {
        state_.store(PickState::Idle);
        return PickState::Failed;
    }

    return state;
}

bool WindowPicker::pending() const {
    return state_.load() == PickState::Pending;
}

void WindowPicker::pick_thread() {
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        state_.store(PickState::Failed);
        return;
    }

    const Window root = DefaultRootWindow(display);
    Cursor cursor = XCreateFontCursor(display, XC_crosshair);
    const int pointer_grab = XGrabPointer(display, root, False, ButtonPressMask,
                                          GrabModeAsync, GrabModeAsync, None, cursor,
                                          CurrentTime);
    const int keyboard_grab = XGrabKeyboard(display, root, False, GrabModeAsync,
                                            GrabModeAsync, CurrentTime);

    if (pointer_grab != GrabSuccess) {
        if (keyboard_grab == GrabSuccess) {
            XUngrabKeyboard(display, CurrentTime);
        }
        if (cursor != None) {
            XFreeCursor(display, cursor);
        }
        XCloseDisplay(display);
        state_.store(PickState::Failed);
        return;
    }

    bool done = false;
    bool cancelled = false;
    Window selected = None;

    while (!done && !stop_.load()) {
        while (XPending(display) > 0) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == ButtonPress) {
                selected = event.xbutton.subwindow;
                if (selected == None) {
                    selected = root;
                } else {
                    selected = deepest_child(display, selected);
                }
                done = true;
                break;
            }

            if (event.type == KeyPress) {
                const KeySym symbol = XLookupKeysym(&event.xkey, 0);
                if (symbol == XK_Escape) {
                    cancelled = true;
                    done = true;
                    break;
                }
            }
        }

        if (!done) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    if (keyboard_grab == GrabSuccess) {
        XUngrabKeyboard(display, CurrentTime);
    }
    XUngrabPointer(display, CurrentTime);
    if (cursor != None) {
        XFreeCursor(display, cursor);
    }

    if (cancelled || selected == None || selected == root) {
        XCloseDisplay(display);
        state_.store(PickState::Idle);
        return;
    }

    const Window client = static_cast<Window>(x11::top_level_client(display, selected));
    if (client == 0) {
        XCloseDisplay(display);
        state_.store(PickState::Failed);
        return;
    }

    WindowInfo info;
    info.id_ = client;
    info.class_name_ = x11::class_name(display, client);
    info.title_ = x11::window_title(display, client);
    info.self_ = x11::window_pid(display, client) == static_cast<long>(getpid());
    XCloseDisplay(display);

    const bool empty = info.empty();
    const bool self = info.self_;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        info_ = std::move(info);
    }

    state_.store(empty ? PickState::Failed
                       : (self ? PickState::Self : PickState::Selected));
}
