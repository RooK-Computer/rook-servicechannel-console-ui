#include "screens/VpnErrorScreen.hpp"

#include "app/Intent.hpp"

namespace rook::ui::screens {

std::string_view VpnErrorScreen::id() const { return "vpn-error"; }

std::string_view VpnErrorScreen::title() const { return "Verbindung fehlgeschlagen"; }

render::ScreenModel VpnErrorScreen::model(const ScreenContext& context) const {
  const auto message = context.param_or("message", "Verbindung konnte nicht aufgebaut werden");
  return render::ScreenModel{
      .screen_id = "vpn-error",
      .title = "Verbindung fehlgeschlagen",
      .body_lines = {message},
       .actions = components::ActionRow{
           .id = "vpn-error-actions",
           .items = {
               components::ActionItem{
                   .id = "vpn-error-retry",
                    .label = "WLAN wechseln",
                   .intent = app::navigate_to("wifi-scan"),
               },
              components::ActionItem{
                  .id = "vpn-error-exit",
                  .label = "Abbrechen und Beenden",
                  .intent = app::close_app(),
               },
           },
       },
   };
}

}  // namespace rook::ui::screens
