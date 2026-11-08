#pragma once

#include "event_key.hh"
#include "target_window.hh"

#include <filesystem>
#include <optional>

class Config {
public:
    std::optional<EventKey> event_;
    std::optional<TargetWindow> target_;

    static Config load(const std::filesystem::path &path = default_path());
    void save(const std::filesystem::path &path = default_path()) const;

    static std::filesystem::path default_path();
};
