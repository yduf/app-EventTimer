#include "x11_window.hh"

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <vector>

namespace {

bool has_wm_state(Display *display, Window window) {
    const Atom wm_state = XInternAtom(display, "WM_STATE", False);
    Atom actual_type = None;
    int actual_format = 0;
    unsigned long count = 0;
    unsigned long remaining = 0;
    unsigned char *data = nullptr;

    const int status = XGetWindowProperty(display, window, wm_state, 0, 0, False,
                                          AnyPropertyType, &actual_type, &actual_format,
                                          &count, &remaining, &data);
    if (data != nullptr) {
        XFree(data);
    }
    return status == Success && actual_type != None;
}

void collect_clients(Display *display, Window window, std::vector<Window> &clients) {
    if (has_wm_state(display, window)) {
        clients.push_back(window);
        return;
    }

    Window root = 0;
    Window parent = 0;
    Window *children = nullptr;
    unsigned int count = 0;
    if (!XQueryTree(display, window, &root, &parent, &children, &count)) {
        return;
    }

    for (unsigned int index = 0; index < count; ++index) {
        collect_clients(display, children[index], clients);
    }
    if (children != nullptr) {
        XFree(children);
    }
}

}

namespace x11 {

unsigned long top_level_client(Display *display, unsigned long window) {
    const Window root = DefaultRootWindow(display);
    Window current = static_cast<Window>(window);

    while (current != 0 && current != root) {
        if (has_wm_state(display, current)) {
            return current;
        }

        Window root_return = 0;
        Window parent_return = 0;
        Window *children = nullptr;
        unsigned int count = 0;
        if (!XQueryTree(display, current, &root_return, &parent_return, &children, &count)) {
            break;
        }
        if (children != nullptr) {
            XFree(children);
        }
        if (parent_return == 0 || parent_return == root) {
            break;
        }
        current = parent_return;
    }

    return current == root ? 0 : current;
}

std::string class_name(Display *display, unsigned long window) {
    XClassHint hint{};
    std::string result;

    if (XGetClassHint(display, static_cast<Window>(window), &hint)) {
        if (hint.res_class != nullptr) {
            result = hint.res_class;
        }
        if (hint.res_name != nullptr) {
            XFree(hint.res_name);
        }
        if (hint.res_class != nullptr) {
            XFree(hint.res_class);
        }
    }

    return result;
}

std::string window_title(Display *display, unsigned long window) {
    const Atom net_wm_name = XInternAtom(display, "_NET_WM_NAME", False);
    const Atom utf8_string = XInternAtom(display, "UTF8_STRING", False);
    Atom actual_type = None;
    int actual_format = 0;
    unsigned long count = 0;
    unsigned long remaining = 0;
    unsigned char *data = nullptr;
    std::string result;

    if (XGetWindowProperty(display, static_cast<Window>(window), net_wm_name, 0, 1024,
                           False, utf8_string, &actual_type, &actual_format, &count,
                           &remaining, &data) == Success &&
        data != nullptr) {
        result.assign(reinterpret_cast<const char *>(data), count);
        XFree(data);
    }

    if (result.empty()) {
        char *name = nullptr;
        if (XFetchName(display, static_cast<Window>(window), &name) && name != nullptr) {
            result = name;
            XFree(name);
        }
    }

    return result;
}

long window_pid(Display *display, unsigned long window) {
    const Atom pid_atom = XInternAtom(display, "_NET_WM_PID", False);
    Atom actual_type = None;
    int actual_format = 0;
    unsigned long count = 0;
    unsigned long remaining = 0;
    unsigned char *data = nullptr;
    long pid = 0;

    if (XGetWindowProperty(display, static_cast<Window>(window), pid_atom, 0, 1, False,
                           XA_CARDINAL, &actual_type, &actual_format, &count, &remaining,
                           &data) == Success &&
        data != nullptr) {
        if (actual_format == 32 && count >= 1) {
            pid = static_cast<long>(reinterpret_cast<unsigned long *>(data)[0]);
        }
        XFree(data);
    }

    return pid;
}

std::optional<unsigned long> find_window(Display *display, const TargetWindow &target) {
    std::vector<Window> clients;
    collect_clients(display, DefaultRootWindow(display), clients);

    std::vector<Window> class_matches;
    for (const Window window : clients) {
        if (!target.class_name_.empty() && class_name(display, window) == target.class_name_) {
            class_matches.push_back(window);
        }
    }

    if (!class_matches.empty()) {
        if (!target.title_.empty()) {
            for (const Window window : class_matches) {
                if (window_title(display, window) == target.title_) {
                    return window;
                }
            }
        }
        return class_matches.front();
    }

    if (!target.title_.empty()) {
        for (const Window window : clients) {
            if (window_title(display, window) == target.title_) {
                return window;
            }
        }
    }

    return std::nullopt;
}

}
