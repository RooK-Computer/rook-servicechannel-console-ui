#include "screens/WifiErrorScreen.hpp"

#include "app/Intent.hpp"

namespace rook::ui::screens {

std::string_view WifiErrorScreen::id() const { return "wifi-error"; }

std::string_view WifiErrorScreen::title() const { return "WLAN-Verbindung fehlgeschlagen"; }

render::ScreenModel WifiErrorScreen::model(const ScreenContext& context) const {
  const auto message = context.param_or("message", "Verbindung zum WLAN konnte nicht hergestellt werden");
  return render::ScreenModel{
      .screen_id = "wifi-error",
      .title = "WLAN-Verbindung fehlgeschlagen",
      .body_lines = {message},
       .actions = components::ActionRow{
           .id = "wifi-error-actions",
           .items = {
               components::ActionItem{
                   .id = "wifi-error-back",
                   .label = "WLAN erneut suchen",
                   .intent = app::navigate_to("wifi-scan"),
               },
              components::ActionItem{
                  .id = "wifi-error-exit",
                  .label = "Abbrechen und Beenden",
                  .intent = app::close_app(),
               },
           },
       },
   };
}

}  // namespace rook::ui::screens
