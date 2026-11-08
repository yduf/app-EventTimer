#include "myFunc/doctest.h"

#include "target_window.hh"

#include <nlohmann/json.hpp>

TEST_CASE("target window json round trip") {
    const TargetWindow original{"obs", "OBS 33.0.1"};
    const nlohmann::json json = original;

    CHECK(json.at("class") == "obs");
    CHECK(json.at("title") == "OBS 33.0.1");

    const auto parsed = json.get<TargetWindow>();
    CHECK(parsed == original);
}

TEST_CASE("target window display includes class and title") {
    CHECK(TargetWindow{"obs", "OBS"}.display() == "obs — OBS");
    CHECK(TargetWindow{"", "OBS"}.display() == "OBS");
    CHECK(TargetWindow{"obs", ""}.display() == "obs");
    CHECK(TargetWindow{}.display().empty());
}

TEST_CASE("empty target window detection") {
    CHECK(TargetWindow{}.empty());
    CHECK_FALSE(TargetWindow{"obs", ""}.empty());
    CHECK_FALSE(TargetWindow{"", "OBS"}.empty());
}
