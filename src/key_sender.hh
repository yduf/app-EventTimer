#pragma once

#include "event_key.hh"
#include "target_window.hh"

#include <optional>
#include <string_view>

enum class SendResult {
    Sent,
    NoEvent,
    SelfFocused,
    TargetNotFound,
    Unavailable,
    Failed,
};

class KeySender {
public:
    SendResult send(const EventKey &key,
                    const std::optional<TargetWindow> &target = std::nullopt) const;
};

std::string_view describe(SendResult result);
