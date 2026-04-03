#include "theme/Theme.hpp"

#include <sstream>

namespace rook::ui::theme {

const Theme& default_theme() {
  static const Theme theme{
      .name = "rook-retro",
      .background_hex = "#ffdc00",
      .focus_hex = "#9000ff",
      .text_hex = "#1a1a1a",
      .panel_hex = "#202020",
      .spacing_unit_px = 16,
      .focus_outline_px = 4,
      .typography = TypographyScale{},
  };

  return theme;
}

std::string describe_theme(const Theme& theme) {
  std::ostringstream stream;
  stream << "theme=" << theme.name
         << " bg=" << theme.background_hex
         << " focus=" << theme.focus_hex
         << " text=" << theme.text_hex
         << " title=" << theme.typography.screen_title_px << "px"
         << " body=" << theme.typography.body_text_px << "px"
         << " button=" << theme.typography.button_text_px << "px";
  return stream.str();
}

}  // namespace rook::ui::theme
