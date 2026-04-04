#include "screens/KeyboardScreen.hpp"

#include "app/Intent.hpp"

namespace rook::ui::screens {

std::string_view KeyboardScreen::id() const { return "keyboard"; }

std::string_view KeyboardScreen::title() const { return "WLAN-Passwort eingeben"; }

render::ScreenModel KeyboardScreen::model(const ScreenContext& context) const {
  const auto ssid = context.param_or("ssid", "RooK-Setup");
  const auto password = context.param_or("password", "hunter2");

  return render::ScreenModel{
      .screen_id = "keyboard",
      .title = "WLAN-Passwort eingeben",
      .body_lines = {
          "SSID: " + ssid,
          "Passwort",
          password,
          "B loescht Zeichen, bei leerer Eingabe geht es zurueck.",
      },
      .actions = components::ActionRow{
          .id = "keyboard-actions",
          .items = {
              components::ActionItem{
                  .id = "keyboard-connect",
                  .label = "Verbinden",
                  .intent = app::navigate_to("wifi-wait", {{"ssid", ssid}}),
              },
              components::ActionItem{
                  .id = "keyboard-back",
                  .label = "Zurueck",
                  .intent = app::go_back(),
              },
          },
      },
      .footer_hint = "QWERTZ-On-Screen-Keyboard folgt als naechster Ausbauschritt.",
  };
}

}  // namespace rook::ui::screens
