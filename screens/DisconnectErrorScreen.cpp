#include "screens/DisconnectErrorScreen.hpp"

#include "app/Intent.hpp"

namespace rook::ui::screens {

std::string_view DisconnectErrorScreen::id() const { return "disconnect-error"; }

std::string_view DisconnectErrorScreen::title() const { return "Trennen fehlgeschlagen"; }

render::ScreenModel DisconnectErrorScreen::model(const ScreenContext& context) const {
  const auto message = context.param_or("message", "Service-Verbindung konnte nicht sauber getrennt werden");
  return render::ScreenModel{
      .screen_id = "disconnect-error",
      .title = "Trennen fehlgeschlagen",
      .body_lines = {message},
      .actions = components::ActionRow{
          .id = "disconnect-error-actions",
          .items = {
              components::ActionItem{
                  .id = "disconnect-error-home",
                  .label = "Zurueck zum Start",
                  .intent = app::navigate_to("welcome"),
              },
              components::ActionItem{
                  .id = "disconnect-error-exit",
                  .label = "Beenden",
                  .intent = app::close_app(),
              },
          },
      },
  };
}

}  // namespace rook::ui::screens
