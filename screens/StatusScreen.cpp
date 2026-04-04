#include "screens/StatusScreen.hpp"

#include "app/Intent.hpp"

namespace rook::ui::screens {

std::string_view StatusScreen::id() const { return "status"; }

std::string_view StatusScreen::title() const { return "RooK Service"; }

render::ScreenModel StatusScreen::model(const ScreenContext& context) const {
  const auto pin = context.param_or("pin", "4739");

  render::ScreenModel model{
      .screen_id = "status",
      .title = "RooK Service",
      .body_lines = {"Service-PIN", pin},
      .actions = components::ActionRow{
          .id = "status-actions",
          .items = {
              components::ActionItem{
                  .id = "status-disconnect",
                  .label = "Trennen",
                  .intent = app::navigate_to("status", {{"dialog", "disconnect"}, {"pin", pin}}),
              },
              components::ActionItem{
                  .id = "status-exit",
                  .label = "Beenden",
                  .intent = app::close_app(),
              },
          },
      },
      .footer_hint = "Aktiver Statusbildschirm mit grosser Service-PIN.",
  };

  if (context.param("dialog") == std::optional<std::string_view>{"disconnect"}) {
    model.dialog = components::DialogModel{
        .id = "disconnect-dialog",
        .title = "Service-Verbindung trennen",
        .body_lines = {"Service-Verbindung wirklich trennen?"},
        .actions = {
            components::ActionItem{
                .id = "disconnect-confirm",
                .label = "Trennen",
                .intent = app::navigate_to("welcome"),
            },
            components::ActionItem{
                .id = "disconnect-back",
                .label = "Zurueck",
                .intent = app::go_back(),
            },
        },
    };
  }

  return model;
}

}  // namespace rook::ui::screens
