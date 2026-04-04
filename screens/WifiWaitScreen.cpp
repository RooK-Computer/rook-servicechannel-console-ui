#include "screens/WifiWaitScreen.hpp"

namespace rook::ui::screens {

std::string_view WifiWaitScreen::id() const { return "wifi-wait"; }

std::string_view WifiWaitScreen::title() const { return "Mit WLAN verbinden"; }

render::ScreenModel WifiWaitScreen::model(const ScreenContext& context) const {
  const auto ssid = context.param("ssid");

  render::ScreenModel model{
      .screen_id = "wifi-wait",
      .title = "Mit WLAN verbinden",
      .body_lines = {"Verbindung zum WLAN wird hergestellt"},
      .show_spinner = true,
      .allow_back = false,
  };

  if (ssid.has_value()) {
    model.body_lines.insert(model.body_lines.begin(), "SSID: " + std::string(*ssid));
  }

  return model;
}

}  // namespace rook::ui::screens
