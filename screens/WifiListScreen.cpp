#include "screens/WifiListScreen.hpp"

#include <algorithm>
#include <array>

#include "app/Intent.hpp"

namespace rook::ui::screens {

std::string_view WifiListScreen::id() const { return "wifi-list"; }

std::string_view WifiListScreen::title() const { return "Mit WLAN verbinden"; }

render::ScreenModel WifiListScreen::model(const ScreenContext& context) const {
  if (context.flag("empty")) {
    return render::ScreenModel{
        .screen_id = "wifi-list",
        .title = "Mit WLAN verbinden",
        .list = components::ListSection{
            .id = "wifi-list",
            .empty_state_text = "Keine WLAN-Netze gefunden",
        },
        .actions = components::ActionRow{
            .id = "wifi-empty-actions",
            .items = {
                components::ActionItem{
                    .id = "wifi-empty-exit",
                    .label = "Abbrechen und Beenden",
                    .intent = app::close_app(),
                },
            },
        },
        .footer_hint = "Mit Stick oder Schultertasten scrollen",
    };
  }

  std::array<std::string, 10> names = {
      "Atelier-24G",
      "BalkonMesh",
      "Dachboden",
      "Gastnetz",
      "Kellerwerkstatt",
      "Nachbarhaus-2G",
      "RooK-Setup",
      "Servicechannel-Lab",
      "Wohnzimmer-5G",
      "Werkraum",
  };
  std::sort(names.begin(), names.end());

  std::vector<components::ListItem> items;
  items.reserve(names.size());
  for (const auto& name : names) {
    items.push_back(components::ListItem{
        .id = "wifi-" + name,
        .primary_text = name,
        .intent = app::navigate_to("keyboard", {{"ssid", name}}),
    });
  }

  return render::ScreenModel{
      .screen_id = "wifi-list",
      .title = "Mit WLAN verbinden",
      .list = components::ListSection{
          .id = "wifi-list",
          .title = "Verfuegbare WLAN-Netze",
          .items = std::move(items),
      },
      .actions = components::ActionRow{
          .id = "wifi-actions",
          .items = {
              components::ActionItem{
                  .id = "wifi-back",
                  .label = "Zurueck",
                  .intent = app::go_back(),
              },
          },
      },
      .footer_hint = "Mit Stick oder Schultertasten scrollen",
  };
}

}  // namespace rook::ui::screens
