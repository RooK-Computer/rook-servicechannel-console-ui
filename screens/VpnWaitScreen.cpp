#include "screens/VpnWaitScreen.hpp"

namespace rook::ui::screens {

std::string_view VpnWaitScreen::id() const { return "vpn-wait"; }

std::string_view VpnWaitScreen::title() const { return "Sichere Verbindung"; }

render::ScreenModel VpnWaitScreen::model(const ScreenContext& context) const {
  (void)context;
  return render::ScreenModel{
      .screen_id = "vpn-wait",
      .title = "Sichere Verbindung",
      .body_lines = {"Sichere Verbindung wird aufgebaut"},
      .show_spinner = true,
      .allow_back = false,
  };
}

}  // namespace rook::ui::screens
