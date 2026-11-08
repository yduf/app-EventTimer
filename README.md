# Event Timer

A small countdown timer with a native UI, written in C++20 with
[Slint](https://slint.dev) and built with meson.

## Features

- Set a duration with hours / minutes / seconds steppers (0-23, 0-59, 0-59).
- Enter an event key that will later be sent to the system when the timer expires.
- Start, pause, resume and reset the countdown.
- Remaining time is displayed as `HH:MM:SS` and updated every second.
- When the countdown reaches zero, a modal dialog shows the event key.

Sending the event key to the system is out of scope for this story: the key is
only captured and displayed.

## Prerequisites

- A C++20 compiler (g++ 13 or newer, or clang 16 or newer)
- meson >= 1.1 and ninja
- ruby (standard library only) for the Slint download script
- `myFunc/doctest.h` for the unit tests (optional, tests are skipped when absent)

## Slint

The Slint C++ library is not packaged on most distributions, so the project uses
the official prebuilt binary release, pinned to `1.18.1`:

```sh
ruby scripts/fetch_slint.rb           # download + verify + unpack
ruby scripts/fetch_slint.rb --force   # re-download
```

The script verifies the tarball against its SHA-256 and unpacks it into
`subprojects/slint-cpp` (`bin/slint-compiler`, `include/slint`, `lib/libslint_cpp.so`).
`meson setup` runs the script automatically when that directory is missing.

The package can also be provided elsewhere with `-Dslint_dir=/path/to/slint-cpp`.

## Build

```sh
meson setup build
meson compile -C build
```

The `.slint` file is compiled to `app.h` / `app.cpp` at build time by
`slint-compiler`, so no `.slint` file is read at run time. The executable carries
an rpath to the Slint library directory, so `LD_LIBRARY_PATH` is not needed.

## Run

```sh
./build/event-timer
```

## Test

```sh
meson test -C build
```

Unit tests cover the countdown logic (formatting, ticking, pause/resume, reset and
expiry). The doctest header is looked up in `-Dmyfunc_dir` (default
`/home/yves/DEV/cpp`); set `-Dtests=false` to skip the test target entirely.

## Layout

```
ui/app.slint              Slint UI description
src/main.cc               wiring between the UI and the timer logic
src/countdown_timer.hh/.cc  countdown state machine (steady_clock based)
src/countdown_timer_test.cc doctest unit tests
scripts/fetch_slint.rb    Slint C++ binary package downloader
```
