#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "myFunc/doctest.h"

#include "countdown_timer.hh"

#include <chrono>

namespace {

using Clock = CountdownTimer::Clock;
using std::chrono::milliseconds;
using std::chrono::seconds;

const Clock::time_point base = Clock::time_point{} + seconds{1000};

}

TEST_CASE("format pads to two digits") {
    CHECK(CountdownTimer::format(seconds{0}) == "00:00:00");
    CHECK(CountdownTimer::format(seconds{5}) == "00:00:05");
    CHECK(CountdownTimer::format(seconds{65}) == "00:01:05");
    CHECK(CountdownTimer::format(seconds{3661}) == "01:01:01");
    CHECK(CountdownTimer::format(seconds{36000}) == "10:00:00");
    CHECK(CountdownTimer::format(seconds{-5}) == "00:00:00");
}

TEST_CASE("counts down one second at a time") {
    CountdownTimer timer{seconds{5}};
    CHECK(timer.remaining() == seconds{5});
    CHECK(timer.remaining_text() == "00:00:05");

    timer.start(base);
    CHECK(timer.running());

    timer.update(base);
    CHECK(timer.remaining() == seconds{5});

    timer.update(base + milliseconds{999});
    CHECK(timer.remaining() == seconds{5});

    timer.update(base + milliseconds{1000});
    CHECK(timer.remaining() == seconds{4});

    timer.update(base + milliseconds{4900});
    CHECK(timer.remaining() == seconds{1});

    timer.update(base + milliseconds{5000});
    CHECK(timer.remaining() == seconds{0});
    CHECK(timer.remaining_text() == "00:00:00");
    CHECK(timer.expired());
    CHECK_FALSE(timer.running());
}

TEST_CASE("expires only once") {
    CountdownTimer timer{seconds{2}};
    timer.start(base);
    timer.update(base + seconds{2});
    CHECK(timer.expired());

    timer.update(base + seconds{30});
    CHECK(timer.expired());
    CHECK(timer.remaining() == seconds{0});

    timer.start(base + seconds{31});
    CHECK_FALSE(timer.running());
}

TEST_CASE("pause freezes and start resumes") {
    CountdownTimer timer{seconds{10}};
    CHECK_FALSE(timer.started());

    timer.start(base);
    timer.update(base + seconds{3});
    CHECK(timer.remaining() == seconds{7});

    timer.pause(base + seconds{3});
    CHECK_FALSE(timer.running());
    CHECK(timer.started());
    CHECK(timer.remaining() == seconds{7});

    timer.update(base + seconds{8});
    CHECK(timer.remaining() == seconds{7});

    timer.start(base + seconds{8});
    timer.update(base + seconds{10});
    CHECK(timer.remaining() == seconds{5});

    timer.reset();
    CHECK_FALSE(timer.started());
}

TEST_CASE("reset restores the initial value") {
    CountdownTimer timer{seconds{30}};
    timer.start(base);
    timer.update(base + seconds{10});
    CHECK(timer.remaining() == seconds{20});

    timer.reset();
    CHECK(timer.remaining() == seconds{30});
    CHECK_FALSE(timer.running());
    CHECK_FALSE(timer.expired());

    timer.set_initial(seconds{90});
    CHECK(timer.initial() == seconds{90});
    CHECK(timer.remaining() == seconds{90});

    timer.set_initial(seconds{-5});
    CHECK(timer.remaining() == seconds{0});
}

TEST_CASE("a zero duration never starts") {
    CountdownTimer timer{seconds{0}};
    timer.start(base);
    CHECK_FALSE(timer.running());
    timer.update(base + seconds{1});
    CHECK_FALSE(timer.expired());
}

TEST_CASE("starting an already running timer is a no-op") {
    CountdownTimer timer{seconds{10}};
    timer.start(base);
    timer.start(base + seconds{4});
    timer.update(base + seconds{5});
    CHECK(timer.remaining() == seconds{5});
}

TEST_CASE("changing the initial value discards progress and stops") {
    CountdownTimer timer{seconds{10}};
    timer.start(base);
    timer.update(base + seconds{3});
    CHECK(timer.remaining() == seconds{7});

    timer.set_initial(seconds{20});
    CHECK(timer.initial() == seconds{20});
    CHECK(timer.remaining() == seconds{20});
    CHECK_FALSE(timer.running());
    CHECK_FALSE(timer.started());
    CHECK_FALSE(timer.expired());

    timer.pause(base + seconds{3});
    timer.set_initial(seconds{30});
    CHECK(timer.remaining() == seconds{30});
    CHECK_FALSE(timer.running());
    CHECK_FALSE(timer.started());
}
