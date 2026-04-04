#include "screens/KeyboardScreen.hpp"

#include <utility>
#include <vector>

#include "app/Intent.hpp"

namespace rook::ui::screens {

namespace {

using GridItem = components::GridItem;
using GridRow = components::GridRow;

struct LayoutState {
  bool shift = false;
  bool alt = false;
  bool caps = false;
};

std::string utf8_pop_back(std::string value) {
  while (!value.empty()) {
    const unsigned char byte = static_cast<unsigned char>(value.back());
    value.pop_back();
    if ((byte & 0xC0) != 0x80) {
      break;
    }
  }
  return value;
}

std::string bool_param(bool value) {
  return value ? "true" : "false";
}

app::IntentParams updated_params(
    const ScreenContext& context,
    std::string password,
    const LayoutState& layout,
    std::string focus_key) {
  app::IntentParams params = context.params;
  params["password"] = std::move(password);
  params["shift"] = bool_param(layout.shift);
  params["alt"] = bool_param(layout.alt);
  params["caps"] = bool_param(layout.caps);
  params["focus-key"] = std::move(focus_key);
  return params;
}

app::Intent navigate_keyboard(
    const ScreenContext& context,
    std::string password,
    const LayoutState& layout,
    std::string focus_key) {
  return app::navigate_to("keyboard", updated_params(context, std::move(password), layout, std::move(focus_key)));
}

GridItem character_key(
    const ScreenContext& context,
    std::string id,
    std::string label,
    std::string character,
    const LayoutState& layout) {
  const std::string focus_key = id;
  LayoutState next_layout = layout;
  if (layout.shift) {
    next_layout.shift = false;
  }

  return GridItem{
      .id = std::move(id),
      .label = std::move(label),
      .intent = navigate_keyboard(context, context.param_or("password", "") + character, next_layout, focus_key),
  };
}

GridItem special_key(
    const ScreenContext& context,
    std::string id,
    std::string label,
    LayoutState next_layout,
    std::string next_password,
    std::size_t width,
    bool active) {
  const std::string focus_key = id;
  return GridItem{
      .id = std::move(id),
      .label = std::move(label),
      .intent = navigate_keyboard(context, std::move(next_password), next_layout, focus_key),
      .width = width,
      .active = active,
  };
}

GridRow make_row(std::string id, std::vector<GridItem> items) {
  return GridRow{
      .id = std::move(id),
      .items = std::move(items),
  };
}

std::string layout_label(const LayoutState& layout) {
  std::string label = "Layout: ";
  label += layout.alt ? "Alt" : "QWERTZ";
  label += " | Shift ";
  label += layout.shift ? "an" : "aus";
  label += " | Caps ";
  label += layout.caps ? "an" : "aus";
  return label;
}

std::string apply_case(std::string value, bool uppercase) {
  if (!uppercase) {
    return value;
  }

  if (value == "q") return "Q";
  if (value == "w") return "W";
  if (value == "e") return "E";
  if (value == "r") return "R";
  if (value == "t") return "T";
  if (value == "z") return "Z";
  if (value == "u") return "U";
  if (value == "i") return "I";
  if (value == "o") return "O";
  if (value == "p") return "P";
  if (value == "a") return "A";
  if (value == "s") return "S";
  if (value == "d") return "D";
  if (value == "f") return "F";
  if (value == "g") return "G";
  if (value == "h") return "H";
  if (value == "j") return "J";
  if (value == "k") return "K";
  if (value == "l") return "L";
  if (value == "y") return "Y";
  if (value == "x") return "X";
  if (value == "c") return "C";
  if (value == "v") return "V";
  if (value == "b") return "B";
  if (value == "n") return "N";
  if (value == "m") return "M";
  if (value == "ä") return "Ä";
  if (value == "ö") return "Ö";
  if (value == "ü") return "Ü";
  return value;
}

LayoutState read_layout(const ScreenContext& context) {
  return LayoutState{
      .shift = context.flag("shift"),
      .alt = context.flag("alt"),
      .caps = context.flag("caps"),
  };
}

std::vector<GridRow> build_rows(const ScreenContext& context, const LayoutState& layout) {
  const std::string password = context.param_or("password", "");
  const bool uppercase = layout.shift || layout.caps;

  if (layout.alt) {
    return {
        make_row("keyboard-row-1",
                 {
                     character_key(context, "alt-1", "!", "!", layout),
                     character_key(context, "alt-2", "\"", "\"", layout),
                     character_key(context, "alt-3", "#", "#", layout),
                     character_key(context, "alt-4", "$", "$", layout),
                     character_key(context, "alt-5", "%", "%", layout),
                     character_key(context, "alt-6", "&", "&", layout),
                     character_key(context, "alt-7", "/", "/", layout),
                     character_key(context, "alt-8", "(", "(", layout),
                     character_key(context, "alt-9", ")", ")", layout),
                     character_key(context, "alt-0", "=", "=", layout),
                     character_key(context, "alt-qmark", "?", "?", layout),
                     special_key(context, "backspace", "⌫", layout, utf8_pop_back(password), 2, false),
                 }),
        make_row("keyboard-row-2",
                 {
                     character_key(context, "alt-at", "@", "@", layout),
                     character_key(context, "alt-euro", "EUR", "€", layout),
                     character_key(context, "alt-star", "*", "*", layout),
                     character_key(context, "alt-plus", "+", "+", layout),
                     character_key(context, "alt-minus", "-", "-", layout),
                     character_key(context, "alt-underscore", "_", "_", layout),
                     character_key(context, "alt-colon", ":", ":", layout),
                     character_key(context, "alt-semicolon", ";", ";", layout),
                     character_key(context, "alt-comma", ",", ",", layout),
                     character_key(context, "alt-dot", ".", ".", layout),
                     character_key(context, "alt-slash", "\\", "\\", layout),
                     character_key(context, "alt-tilde", "~", "~", layout),
                 }),
        make_row("keyboard-row-3",
                 {
                     character_key(context, "alt-lbrace", "{", "{", layout),
                     character_key(context, "alt-rbrace", "}", "}", layout),
                     character_key(context, "alt-lbracket", "[", "[", layout),
                     character_key(context, "alt-rbracket", "]", "]", layout),
                     character_key(context, "alt-lt", "<", "<", layout),
                     character_key(context, "alt-gt", ">", ">", layout),
                     character_key(context, "alt-pipe", "|", "|", layout),
                     character_key(context, "alt-caret", "^", "^", layout),
                     character_key(context, "alt-grave", "`", "`", layout),
                     character_key(context, "alt-quote", "'", "'", layout),
                     character_key(context, "alt-space-mini", ".", ".", layout),
                 }),
        make_row("keyboard-row-4",
                 {
                     special_key(context, "shift", "Shift", LayoutState{.shift = !layout.shift, .alt = layout.alt, .caps = layout.caps}, password, 2, layout.shift),
                     special_key(context, "alt", "Alt", LayoutState{.shift = false, .alt = !layout.alt, .caps = layout.caps}, password, 2, layout.alt),
                     special_key(context, "caps", "Caps", LayoutState{.shift = layout.shift, .alt = layout.alt, .caps = !layout.caps}, password, 2, layout.caps),
                     special_key(context, "space", "Leer", layout, password + " ", 4, false),
                 }),
    };
  }

  return {
      make_row("keyboard-row-1",
               {
                   character_key(context, "digit-1", layout.shift ? "!" : "1", layout.shift ? "!" : "1", layout),
                   character_key(context, "digit-2", layout.shift ? "\"" : "2", layout.shift ? "\"" : "2", layout),
                   character_key(context, "digit-3", layout.shift ? "§" : "3", layout.shift ? "§" : "3", layout),
                   character_key(context, "digit-4", layout.shift ? "$" : "4", layout.shift ? "$" : "4", layout),
                   character_key(context, "digit-5", layout.shift ? "%" : "5", layout.shift ? "%" : "5", layout),
                   character_key(context, "digit-6", layout.shift ? "&" : "6", layout.shift ? "&" : "6", layout),
                   character_key(context, "digit-7", layout.shift ? "/" : "7", layout.shift ? "/" : "7", layout),
                   character_key(context, "digit-8", layout.shift ? "(" : "8", layout.shift ? "(" : "8", layout),
                   character_key(context, "digit-9", layout.shift ? ")" : "9", layout.shift ? ")" : "9", layout),
                   character_key(context, "digit-0", layout.shift ? "=" : "0", layout.shift ? "=" : "0", layout),
                   character_key(context, "digit-ss", layout.shift ? "?" : "ß", layout.shift ? "?" : "ß", layout),
                   special_key(context, "backspace", "⌫", layout, utf8_pop_back(password), 2, false),
               }),
      make_row("keyboard-row-2",
               {
                   character_key(context, "q", apply_case("q", uppercase), apply_case("q", uppercase), layout),
                   character_key(context, "w", apply_case("w", uppercase), apply_case("w", uppercase), layout),
                   character_key(context, "e", apply_case("e", uppercase), apply_case("e", uppercase), layout),
                   character_key(context, "r", apply_case("r", uppercase), apply_case("r", uppercase), layout),
                   character_key(context, "t", apply_case("t", uppercase), apply_case("t", uppercase), layout),
                   character_key(context, "z", apply_case("z", uppercase), apply_case("z", uppercase), layout),
                   character_key(context, "u", apply_case("u", uppercase), apply_case("u", uppercase), layout),
                   character_key(context, "i", apply_case("i", uppercase), apply_case("i", uppercase), layout),
                   character_key(context, "o", apply_case("o", uppercase), apply_case("o", uppercase), layout),
                   character_key(context, "p", apply_case("p", uppercase), apply_case("p", uppercase), layout),
                   character_key(context, "ue", apply_case("ü", uppercase), apply_case("ü", uppercase), layout),
                   character_key(context, "plus", layout.shift ? "*" : "+", layout.shift ? "*" : "+", layout),
               }),
      make_row("keyboard-row-3",
               {
                   character_key(context, "a", apply_case("a", uppercase), apply_case("a", uppercase), layout),
                   character_key(context, "s", apply_case("s", uppercase), apply_case("s", uppercase), layout),
                   character_key(context, "d", apply_case("d", uppercase), apply_case("d", uppercase), layout),
                   character_key(context, "f", apply_case("f", uppercase), apply_case("f", uppercase), layout),
                   character_key(context, "g", apply_case("g", uppercase), apply_case("g", uppercase), layout),
                   character_key(context, "h", apply_case("h", uppercase), apply_case("h", uppercase), layout),
                   character_key(context, "j", apply_case("j", uppercase), apply_case("j", uppercase), layout),
                   character_key(context, "k", apply_case("k", uppercase), apply_case("k", uppercase), layout),
                   character_key(context, "l", apply_case("l", uppercase), apply_case("l", uppercase), layout),
                   character_key(context, "oe", apply_case("ö", uppercase), apply_case("ö", uppercase), layout),
                   character_key(context, "ae", apply_case("ä", uppercase), apply_case("ä", uppercase), layout),
                   character_key(context, "hash", layout.shift ? "'" : "#", layout.shift ? "'" : "#", layout),
               }),
      make_row("keyboard-row-4",
               {
                   special_key(context, "shift", "Shift", LayoutState{.shift = !layout.shift, .alt = layout.alt, .caps = layout.caps}, password, 2, layout.shift),
                   character_key(context, "lt", layout.shift ? ">" : "<", layout.shift ? ">" : "<", layout),
                   character_key(context, "y", apply_case("y", uppercase), apply_case("y", uppercase), layout),
                   character_key(context, "x", apply_case("x", uppercase), apply_case("x", uppercase), layout),
                   character_key(context, "c", apply_case("c", uppercase), apply_case("c", uppercase), layout),
                   character_key(context, "v", apply_case("v", uppercase), apply_case("v", uppercase), layout),
                   character_key(context, "b", apply_case("b", uppercase), apply_case("b", uppercase), layout),
                   character_key(context, "n", apply_case("n", uppercase), apply_case("n", uppercase), layout),
                   character_key(context, "m", apply_case("m", uppercase), apply_case("m", uppercase), layout),
                   character_key(context, "comma", layout.shift ? ";" : ",", layout.shift ? ";" : ",", layout),
                   character_key(context, "dot", layout.shift ? ":" : ".", layout.shift ? ":" : ".", layout),
                   character_key(context, "dash", layout.shift ? "_" : "-", layout.shift ? "_" : "-", layout),
               }),
      make_row("keyboard-row-5",
               {
                   special_key(context, "alt", "Alt", LayoutState{.shift = false, .alt = !layout.alt, .caps = layout.caps}, password, 2, layout.alt),
                   special_key(context, "caps", "Caps", LayoutState{.shift = layout.shift, .alt = layout.alt, .caps = !layout.caps}, password, 2, layout.caps),
                   special_key(context, "space", "Leer", layout, password + " ", 6, false),
               }),
  };
}

}  // namespace

std::string_view KeyboardScreen::id() const { return "keyboard"; }

std::string_view KeyboardScreen::title() const { return "WLAN-Passwort eingeben"; }

render::ScreenModel KeyboardScreen::model(const ScreenContext& context) const {
  const auto ssid = context.param_or("ssid", "RooK-Setup");
  const auto password = context.param_or("password", "");
  const auto layout = read_layout(context);

  return render::ScreenModel{
      .screen_id = "keyboard",
      .title = "WLAN-Passwort eingeben",
      .actions = components::ActionRow{
          .id = "keyboard-actions",
          .items = {
                components::ActionItem{
                    .id = "keyboard-connect",
                    .label = "Verbinden",
                    .intent = app::navigate_to("wifi-wait", {{"ssid", ssid}, {"password", password}}),
                },
              components::ActionItem{
                  .id = "keyboard-back",
                  .label = "Zurueck",
                   .intent = app::go_back(),
               },
           },
       },
      .keyboard = components::KeyboardModel{
          .id = "keyboard-grid",
          .ssid = ssid,
          .password = password,
          .helper_text = "B loescht das letzte Zeichen. Bei leerer Eingabe geht B zurueck.",
          .layout_label = layout_label(layout),
          .focus_key_id = context.param_or("focus-key", "q"),
          .rows = build_rows(context, layout),
      },
  };
}

}  // namespace rook::ui::screens
