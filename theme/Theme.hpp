#pragma once

#include <string>

namespace rook::ui::theme {

struct TypographyScale {
  int screen_title_px = 56;
  int dialog_title_px = 44;
  int body_text_px = 32;
  int helper_text_px = 24;
  int button_text_px = 36;
  int pin_label_px = 40;
  int pin_digits_px = 240;
};

struct Theme {
  std::string name;
  std::string background_hex;
  std::string focus_hex;
  std::string text_hex;
  std::string panel_hex;
  int spacing_unit_px = 16;
  int focus_outline_px = 4;
  TypographyScale typography;
};

const Theme& default_theme();
std::string describe_theme(const Theme& theme);

}  // namespace rook::ui::theme
