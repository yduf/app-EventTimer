#pragma once

#include <nlohmann/json_fwd.hpp>

#include <optional>
#include <string>
#include <string_view>

struct EventKey {
    std::string key_;
    bool alt_ = false;
    bool control_ = false;
    bool shift_ = false;
    bool meta_ = false;

    static std::optional<EventKey> from_slint_text(std::string_view text, bool alt,
                                                   bool control, bool shift, bool meta);

    bool empty() const;
    std::string display() const;
    std::string xdotool_key() const;
    std::string xdotool_sequence() const;

    friend bool operator==(const EventKey &, const EventKey &) = default;
};

void to_json(nlohmann::json &json, const EventKey &key);
void from_json(const nlohmann::json &json, EventKey &key);
