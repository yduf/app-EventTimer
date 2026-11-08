#pragma once

#include "target_window.hh"

#include <X11/Xlib.h>

#include <optional>
#include <string>

namespace x11 {

// Walk up from `window` to the managed client window (the one carrying
// WM_STATE), or the topmost non-root ancestor when none is found.
unsigned long top_level_client(Display *display, unsigned long window);

std::string class_name(Display *display, unsigned long window);
std::string window_title(Display *display, unsigned long window);

// Returns the _NET_WM_PID of the window, or 0 when unavailable.
long window_pid(Display *display, unsigned long window);

// Find a managed client window matching the stored class/title.
std::optional<unsigned long> find_window(Display *display, const TargetWindow &target);

}
