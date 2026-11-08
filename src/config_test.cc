#include "myFunc/doctest.h"

#include "config.hh"

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path temporary_path(std::string_view name) {
    return std::filesystem::temp_directory_path() / std::string(name);
}

}

TEST_CASE("config saves and loads the recorded event") {
    const auto path = temporary_path("event-timer-config-round-trip.json");
    std::filesystem::remove(path);

    Config config;
    config.event_ = EventKey{"F1", true, false, true, false};
    config.save(path);

    REQUIRE(std::filesystem::exists(path));

    const auto loaded = Config::load(path);
    REQUIRE(loaded.event_);
    CHECK(*loaded.event_ == *config.event_);

    std::filesystem::remove(path);
}

TEST_CASE("missing config has no event") {
    const auto path = temporary_path("event-timer-config-missing.json");
    std::filesystem::remove(path);

    const auto loaded = Config::load(path);
    CHECK_FALSE(loaded.event_);
}

TEST_CASE("malformed config has no event") {
    const auto path = temporary_path("event-timer-config-malformed.json");
    {
        std::ofstream file(path);
        file << "{ this is not json";
    }

    const auto loaded = Config::load(path);
    CHECK_FALSE(loaded.event_);

    std::filesystem::remove(path);
}

TEST_CASE("empty event is not persisted") {
    const auto path = temporary_path("event-timer-config-empty.json");
    std::filesystem::remove(path);

    Config config;
    config.save(path);

    const auto loaded = Config::load(path);
    CHECK_FALSE(loaded.event_);

    std::filesystem::remove(path);
}

TEST_CASE("config saves and loads the target window") {
    const auto path = temporary_path("event-timer-config-target.json");
    std::filesystem::remove(path);

    Config config;
    config.target_ = TargetWindow{"obs", "OBS 33.0.1"};
    config.save(path);

    const auto loaded = Config::load(path);
    REQUIRE(loaded.target_);
    CHECK(*loaded.target_ == *config.target_);
    CHECK_FALSE(loaded.event_);

    std::filesystem::remove(path);
}

TEST_CASE("config saves and loads event and target together") {
    const auto path = temporary_path("event-timer-config-event-target.json");
    std::filesystem::remove(path);

    Config config;
    config.event_ = EventKey{"F1", true, false, true, false};
    config.target_ = TargetWindow{"obs", "OBS 33.0.1"};
    config.save(path);

    const auto loaded = Config::load(path);
    REQUIRE(loaded.event_);
    REQUIRE(loaded.target_);
    CHECK(*loaded.event_ == *config.event_);
    CHECK(*loaded.target_ == *config.target_);

    std::filesystem::remove(path);
}

TEST_CASE("empty target is not persisted") {
    const auto path = temporary_path("event-timer-config-empty-target.json");
    std::filesystem::remove(path);

    Config config;
    config.target_ = TargetWindow{};
    config.save(path);

    const auto loaded = Config::load(path);
    CHECK_FALSE(loaded.target_);

    std::filesystem::remove(path);
}

TEST_CASE("default config path uses the event-timer directory") {
    const auto path = Config::default_path();
    CHECK(path.filename() == "config.json");
    CHECK(path.parent_path().filename() == "event-timer");
}
