#include "config.hh"

#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>

namespace {

std::filesystem::path config_home() {
    if (const char *xdg = std::getenv("XDG_CONFIG_HOME"); xdg != nullptr && *xdg != '\0') {
        return xdg;
    }
    if (const char *home = std::getenv("HOME"); home != nullptr && *home != '\0') {
        return std::filesystem::path(home) / ".config";
    }
    return std::filesystem::temp_directory_path();
}

}

Config Config::load(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file) {
        return {};
    }

    try {
        nlohmann::json json;
        file >> json;

        Config config;

        if (json.contains("event") && json.at("event").is_object()) {
            auto event = json.at("event").get<EventKey>();
            if (!event.empty()) {
                config.event_ = std::move(event);
            }
        }

        if (json.contains("target") && json.at("target").is_object()) {
            auto target = json.at("target").get<TargetWindow>();
            if (!target.empty()) {
                config.target_ = std::move(target);
            }
        }

        return config;
    } catch (const std::exception &error) {
        std::cerr << "event-timer: could not read " << path << ": " << error.what() << '\n';
        return {};
    }
}

void Config::save(const std::filesystem::path &path) const {
    nlohmann::json json;
    if (event_ && !event_->empty()) {
        json["event"] = *event_;
    } else {
        json["event"] = nullptr;
    }

    if (target_ && !target_->empty()) {
        json["target"] = *target_;
    } else {
        json["target"] = nullptr;
    }

    std::error_code error;
    const auto directory = path.parent_path();
    if (!directory.empty()) {
        std::filesystem::create_directories(directory, error);
        if (error) {
            std::cerr << "event-timer: could not create " << directory << ": " << error.message()
                      << '\n';
            return;
        }
    }

    auto temporary = path;
    temporary += ".tmp";

    {
        std::ofstream file(temporary);
        if (!file) {
            std::cerr << "event-timer: could not write " << temporary << '\n';
            return;
        }
        file << json.dump(2) << '\n';
        if (!file) {
            std::cerr << "event-timer: could not write " << temporary << '\n';
            return;
        }
    }

    std::filesystem::rename(temporary, path, error);
    if (error) {
        std::cerr << "event-timer: could not replace " << path << ": " << error.message() << '\n';
        std::filesystem::remove(temporary, error);
    }
}

std::filesystem::path Config::default_path() {
    return config_home() / "event-timer" / "config.json";
}
