#include "screens/VpnErrorScreen.hpp"

#include "app/Intent.hpp"

namespace rook::ui::screens {

std::string_view VpnErrorScreen::id() const { return "vpn-error"; }

std::string_view VpnErrorScreen::title() const { return "Verbindung fehlgeschlagen"; }

render::ScreenModel VpnErrorScreen::model(const ScreenContext& context) const {
  (void)context;
  return render::ScreenModel{
      .screen_id = "vpn-error",
      .title = "Verbindung fehlgeschlagen",
      .body_lines = {"Verbindung konnte nicht aufgebaut werden"},
      .actions = components::ActionRow{
          .id = "vpn-error-actions",
          .items = {
              components::ActionItem{
                  .id = "vpn-error-retry",
                  .label = "WLAN wechseln",
                  .intent = app::navigate_to("wifi-list"),
              },
              components::ActionItem{
                  .id = "vpn-error-exit",
                  .label = "Abbrechen und Beenden",
                  .intent = app::close_app(),
              },
          },
      },
      .footer_hint = "Fehler-Screen fuer fehlgeschlagenen VPN-Aufbau.",
  };
}

}  // namespace rook::ui::screens
