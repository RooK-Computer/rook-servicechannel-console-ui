#include "screens/WifiScanScreen.hpp"

namespace rook::ui::screens {

std::string_view WifiScanScreen::id() const { return "wifi-scan"; }

std::string_view WifiScanScreen::title() const { return "Mit WLAN verbinden"; }

render::ScreenModel WifiScanScreen::model(const ScreenContext& context) const {
  return render::ScreenModel{
      .screen_id = "wifi-scan",
      .title = "Mit WLAN verbinden",
      .body_lines = {"Verfuegbare WLAN-Netze werden gesucht"},
      .show_spinner = true,
      .allow_back = false,
  };
}

}  // namespace rook::ui::screens
