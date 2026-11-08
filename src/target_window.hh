#pragma once

#include <nlohmann/json_fwd.hpp>

#include <string>

struct TargetWindow {
    std::string class_name_;
    std::string title_;

    bool empty() const;
    std::string display() const;

    friend bool operator==(const TargetWindow &, const TargetWindow &) = default;
};

void to_json(nlohmann::json &json, const TargetWindow &target);
void from_json(const nlohmann::json &json, TargetWindow &target);
