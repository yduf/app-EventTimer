#include "target_window.hh"

#include <nlohmann/json.hpp>

bool TargetWindow::empty() const {
    return class_name_.empty() && title_.empty();
}

std::string TargetWindow::display() const {
    if (!class_name_.empty() && !title_.empty()) {
        return class_name_ + " — " + title_;
    }
    if (!title_.empty()) {
        return title_;
    }
    return class_name_;
}

void to_json(nlohmann::json &json, const TargetWindow &target) {
    json = nlohmann::json{
        {"class", target.class_name_},
        {"title", target.title_},
    };
}

void from_json(const nlohmann::json &json, TargetWindow &target) {
    target.class_name_ = json.value("class", std::string{});
    target.title_ = json.value("title", std::string{});
}
