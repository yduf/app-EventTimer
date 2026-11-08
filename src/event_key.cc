#include "event_key.hh"

#include <nlohmann/json.hpp>

#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <string>
#include <string_view>

namespace {

struct SpecialKey {
    std::uint32_t codepoint;
    std::string_view name;
    std::string_view xdotool_name;
};

constexpr SpecialKey special_keys[] = {
    {0x0008, "Backspace", "BackSpace"},
    {0x0009, "Tab", "Tab"},
    {0x000A, "Return", "Return"},
    {0x000D, "Return", "Return"},
    {0x0019, "Backtab", "ISO_Left_Tab"},
    {0x001B, "Escape", "Escape"},
    {0x0020, "Space", "space"},
    {0x007F, "Delete", "Delete"},
    {0xF700, "Up", "Up"},
    {0xF701, "Down", "Down"},
    {0xF702, "Left", "Left"},
    {0xF703, "Right", "Right"},
    {0xF727, "Insert", "Insert"},
    {0xF729, "Home", "Home"},
    {0xF72B, "End", "End"},
    {0xF72C, "PageUp", "Page_Up"},
    {0xF72D, "PageDown", "Page_Down"},
    {0xF72F, "ScrollLock", "Scroll_Lock"},
    {0xF730, "Pause", "Pause"},
    {0xF731, "SysReq", "Sys_Req"},
    {0xF734, "Stop", "Cancel"},
    {0xF735, "Menu", "Menu"},
    {0xF748, "Back", "BackSpace"},
};

bool is_modifier_codepoint(std::uint32_t codepoint) {
    switch (codepoint) {
    case 0x0010: // Shift
    case 0x0011: // Control
    case 0x0012: // Alt
    case 0x0013: // AltGr
    case 0x0014: // CapsLock
    case 0x0015: // ShiftR
    case 0x0016: // ControlR
    case 0x0017: // Meta
    case 0x0018: // MetaR
        return true;
    default:
        return false;
    }
}

std::optional<std::uint32_t> decode_first_codepoint(std::string_view text) {
    if (text.empty()) {
        return std::nullopt;
    }

    const auto first = static_cast<unsigned char>(text[0]);
    std::uint32_t codepoint = 0;
    std::size_t length = 0;

    if (first < 0x80) {
        codepoint = first;
        length = 1;
    } else if ((first & 0xE0) == 0xC0) {
        codepoint = first & 0x1F;
        length = 2;
    } else if ((first & 0xF0) == 0xE0) {
        codepoint = first & 0x0F;
        length = 3;
    } else if ((first & 0xF8) == 0xF0) {
        codepoint = first & 0x07;
        length = 4;
    } else {
        return std::nullopt;
    }

    if (text.size() < length) {
        return std::nullopt;
    }

    for (std::size_t index = 1; index < length; ++index) {
        const auto continuation = static_cast<unsigned char>(text[index]);
        if ((continuation & 0xC0) != 0x80) {
            return std::nullopt;
        }
        codepoint = (codepoint << 6) | (continuation & 0x3F);
    }

    return codepoint;
}

const SpecialKey *find_special_key(std::uint32_t codepoint) {
    for (const auto &entry : special_keys) {
        if (entry.codepoint == codepoint) {
            return &entry;
        }
    }
    return nullptr;
}

bool is_ascii_letter_or_digit(char character) {
    const auto value = static_cast<unsigned char>(character);
    return std::isalnum(value) != 0;
}

std::string punctuation_name(char character) {
    switch (character) {
    case ' ':
        return "space";
    case '!':
        return "exclam";
    case '"':
        return "quotedbl";
    case '#':
        return "numbersign";
    case '$':
        return "dollar";
    case '%':
        return "percent";
    case '&':
        return "ampersand";
    case '\'':
        return "apostrophe";
    case '(':
        return "parenleft";
    case ')':
        return "parenright";
    case '*':
        return "asterisk";
    case '+':
        return "plus";
    case ',':
        return "comma";
    case '-':
        return "minus";
    case '.':
        return "period";
    case '/':
        return "slash";
    case ':':
        return "colon";
    case ';':
        return "semicolon";
    case '<':
        return "less";
    case '=':
        return "equal";
    case '>':
        return "greater";
    case '?':
        return "question";
    case '@':
        return "at";
    case '[':
        return "bracketleft";
    case '\\':
        return "backslash";
    case ']':
        return "bracketright";
    case '^':
        return "asciicircum";
    case '_':
        return "underscore";
    case '`':
        return "grave";
    case '{':
        return "braceleft";
    case '|':
        return "bar";
    case '}':
        return "braceright";
    case '~':
        return "asciitilde";
    default:
        return {};
    }
}

bool is_function_key_name(std::string_view key) {
    if (key.size() < 2 || key[0] != 'F') {
        return false;
    }
    for (std::size_t index = 1; index < key.size(); ++index) {
        if (std::isdigit(static_cast<unsigned char>(key[index])) == 0) {
            return false;
        }
    }
    const auto number = std::strtol(std::string(key.substr(1)).c_str(), nullptr, 10);
    return number >= 1 && number <= 24;
}

}

std::optional<EventKey> EventKey::from_slint_text(std::string_view text, bool alt,
                                                  bool control, bool shift, bool meta) {
    const auto codepoint = decode_first_codepoint(text);
    if (!codepoint) {
        return std::nullopt;
    }

    if (is_modifier_codepoint(*codepoint)) {
        return std::nullopt;
    }

    if (const auto *special = find_special_key(*codepoint)) {
        return EventKey{std::string(special->name), alt, control, shift, meta};
    }

    if (*codepoint >= 0xF704 && *codepoint <= 0xF71B) {
        return EventKey{std::format("F{}", *codepoint - 0xF704 + 1), alt, control, shift, meta};
    }

    if (*codepoint < 0x20) {
        return std::nullopt;
    }

    if (*codepoint >= 0xE000 && *codepoint <= 0xF8FF) {
        return EventKey{std::format("U+{:04X}", *codepoint), alt, control, shift, meta};
    }

    return EventKey{std::string(text), alt, control, shift, meta};
}

bool EventKey::empty() const {
    return key_.empty();
}

std::string EventKey::display() const {
    std::string result;

    const auto append = [&result](std::string_view part) {
        if (!result.empty()) {
            result += '+';
        }
        result += part;
    };

    if (control_) {
        append("Ctrl");
    }
    if (alt_) {
        append("Alt");
    }
    if (shift_) {
        append("Shift");
    }
    if (meta_) {
        append("Super");
    }

    std::string label = key_;
    if (label.empty()) {
        label = "Unknown";
    } else if (shift_ && label.size() == 1 && label[0] >= 'a' && label[0] <= 'z') {
        label[0] = static_cast<char>(label[0] - 'a' + 'A');
    }

    append(label);
    return result;
}

std::string EventKey::xdotool_key() const {
    if (key_.empty()) {
        return {};
    }

    for (const auto &entry : special_keys) {
        if (key_ == entry.name) {
            return std::string(entry.xdotool_name);
        }
    }

    if (is_function_key_name(key_)) {
        return key_;
    }

    if (key_.size() >= 2 && key_[0] == 'U' && key_[1] == '+') {
        return "U" + key_.substr(2);
    }

    const auto codepoint = decode_first_codepoint(key_);
    if (!codepoint) {
        return {};
    }

    if (*codepoint < 0x80) {
        const auto character = static_cast<char>(*codepoint);
        if (is_ascii_letter_or_digit(character)) {
            return std::string(1, character);
        }
        return punctuation_name(character);
    }

    return std::format("U{:04X}", *codepoint);
}

std::string EventKey::xdotool_sequence() const {
    const auto key = xdotool_key();
    if (key.empty()) {
        return {};
    }

    std::string sequence;
    if (control_) {
        sequence += "ctrl+";
    }
    if (alt_) {
        sequence += "alt+";
    }
    if (shift_) {
        sequence += "shift+";
    }
    if (meta_) {
        sequence += "super+";
    }
    sequence += key;
    return sequence;
}

void to_json(nlohmann::json &json, const EventKey &key) {
    json = nlohmann::json{
        {"key", key.key_},
        {"alt", key.alt_},
        {"control", key.control_},
        {"shift", key.shift_},
        {"meta", key.meta_},
    };
}

void from_json(const nlohmann::json &json, EventKey &key) {
    key.key_ = json.value("key", std::string{});
    key.alt_ = json.value("alt", false);
    key.control_ = json.value("control", false);
    key.shift_ = json.value("shift", false);
    key.meta_ = json.value("meta", false);
}
