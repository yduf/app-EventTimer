#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "myFunc/doctest.h"

#include "event_key.hh"

#include <nlohmann/json.hpp>

#include <string_view>

namespace {

std::string_view slint_key(std::u8string_view text) {
    return {reinterpret_cast<const char *>(text.data()), text.size()};
}

}

TEST_CASE("slint special keys are mapped to canonical names") {
    const auto f1 = EventKey::from_slint_text(slint_key(u8"\uf704"), false, false, false, false);
    REQUIRE(f1);
    CHECK(f1->key_ == "F1");
    CHECK(f1->xdotool_key() == "F1");

    const auto f24 = EventKey::from_slint_text(slint_key(u8"\uf71b"), false, false, false, false);
    REQUIRE(f24);
    CHECK(f24->key_ == "F24");

    const auto return_key =
        EventKey::from_slint_text(slint_key(u8"\u000a"), false, false, false, false);
    REQUIRE(return_key);
    CHECK(return_key->key_ == "Return");
    CHECK(return_key->xdotool_key() == "Return");

    const auto space = EventKey::from_slint_text(slint_key(u8"\u0020"), false, false, false, false);
    REQUIRE(space);
    CHECK(space->key_ == "Space");
    CHECK(space->xdotool_key() == "space");

    const auto escape =
        EventKey::from_slint_text(slint_key(u8"\u001b"), false, false, false, false);
    REQUIRE(escape);
    CHECK(escape->key_ == "Escape");

    const auto backspace =
        EventKey::from_slint_text(slint_key(u8"\u0008"), false, false, false, false);
    REQUIRE(backspace);
    CHECK(backspace->key_ == "Backspace");
    CHECK(backspace->xdotool_key() == "BackSpace");

    const auto delete_key =
        EventKey::from_slint_text(slint_key(u8"\u007f"), false, false, false, false);
    REQUIRE(delete_key);
    CHECK(delete_key->key_ == "Delete");

    const auto page_up =
        EventKey::from_slint_text(slint_key(u8"\uf72c"), false, false, false, false);
    REQUIRE(page_up);
    CHECK(page_up->key_ == "PageUp");
    CHECK(page_up->xdotool_key() == "Page_Up");

    const auto backtab =
        EventKey::from_slint_text(slint_key(u8"\u0019"), false, false, false, false);
    REQUIRE(backtab);
    CHECK(backtab->key_ == "Backtab");
    CHECK(backtab->xdotool_key() == "ISO_Left_Tab");
}

TEST_CASE("modifier keys alone are not recordable") {
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0010"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0011"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0012"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0013"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0014"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0015"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0016"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0017"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text(slint_key(u8"\u0018"), false, false, false, false));
    CHECK_FALSE(EventKey::from_slint_text("", false, false, false, false));
}

TEST_CASE("printable keys keep their character and map to xdotool names") {
    const auto letter = EventKey::from_slint_text("a", false, false, false, false);
    REQUIRE(letter);
    CHECK(letter->key_ == "a");
    CHECK(letter->xdotool_key() == "a");

    const auto uppercase = EventKey::from_slint_text("A", false, false, true, false);
    REQUIRE(uppercase);
    CHECK(uppercase->key_ == "A");
    CHECK(uppercase->xdotool_key() == "A");

    const auto exclamation = EventKey::from_slint_text("!", false, false, true, false);
    REQUIRE(exclamation);
    CHECK(exclamation->key_ == "!");
    CHECK(exclamation->xdotool_key() == "exclam");

    const auto acute = EventKey::from_slint_text(slint_key(u8"\u00e9"), false, false, false, false);
    REQUIRE(acute);
    CHECK(acute->key_ == slint_key(u8"\u00e9"));
    CHECK(acute->xdotool_key() == "U00E9");

    const auto euro = EventKey::from_slint_text(slint_key(u8"\u20ac"), false, false, false, false);
    REQUIRE(euro);
    CHECK(euro->key_ == slint_key(u8"\u20ac"));
    CHECK(euro->xdotool_key() == "U20AC");
}

TEST_CASE("display includes modifiers and key") {
    CHECK(EventKey{"a", false, true, true, false}.display() == "Ctrl+Shift+A");
    CHECK(EventKey{"F1", true, false, false, false}.display() == "Alt+F1");
    CHECK(EventKey{"Space", false, false, false, false}.display() == "Space");
    CHECK(EventKey{"!", false, false, true, false}.display() == "Shift+!");
    CHECK(EventKey{"a", false, false, false, true}.display() == "Super+a");
    CHECK(EventKey{"Return", true, true, true, true}.display() == "Ctrl+Alt+Shift+Super+Return");
}

TEST_CASE("xdotool sequences combine modifiers and key names") {
    CHECK(EventKey{"a", false, true, false, false}.xdotool_sequence() == "ctrl+a");
    CHECK(EventKey{"F1", true, false, false, false}.xdotool_sequence() == "alt+F1");
    CHECK(EventKey{"Space", false, false, false, true}.xdotool_sequence() == "super+space");
    CHECK(EventKey{"!", false, true, true, false}.xdotool_sequence() == "ctrl+shift+exclam");
    CHECK(EventKey{"", true, false, false, false}.xdotool_sequence().empty());
}

TEST_CASE("event key json round trip") {
    const EventKey original{"F1", true, false, true, false};
    const nlohmann::json json = original;

    CHECK(json.at("key") == "F1");
    CHECK(json.at("alt") == true);
    CHECK(json.at("control") == false);
    CHECK(json.at("shift") == true);
    CHECK(json.at("meta") == false);

    const auto parsed = json.get<EventKey>();
    CHECK(parsed == original);
}

TEST_CASE("event key json defaults missing modifiers") {
    const nlohmann::json json = {{"key", "a"}};
    const auto parsed = json.get<EventKey>();
    CHECK(parsed.key_ == "a");
    CHECK_FALSE(parsed.alt_);
    CHECK_FALSE(parsed.control_);
    CHECK_FALSE(parsed.shift_);
    CHECK_FALSE(parsed.meta_);
}
