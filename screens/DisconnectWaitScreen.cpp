#include "screens/DisconnectWaitScreen.hpp"

namespace rook::ui::screens {

std::string_view DisconnectWaitScreen::id() const { return "disconnect-wait"; }

std::string_view DisconnectWaitScreen::title() const { return "Verbindung trennen"; }

render::ScreenModel DisconnectWaitScreen::model(const ScreenContext& context) const {
  (void)context;
  return render::ScreenModel{
      .screen_id = "disconnect-wait",
      .title = "Verbindung trennen",
      .body_lines = {"Service-Verbindung wird getrennt"},
      .show_spinner = true,
      .allow_back = false,
  };
}

}  // namespace rook::ui::screens
