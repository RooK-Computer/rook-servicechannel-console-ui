#include "screens/WifiListScreen.hpp"

#include <algorithm>
#include <array>
#include <sstream>

#include "app/Intent.hpp"

namespace rook::ui::screens {

namespace {

std::vector<std::string> split_networks(std::string_view serialized_networks) {
  std::vector<std::string> networks;
  std::stringstream stream{std::string(serialized_networks)};

  for (std::string line; std::getline(stream, line, '\n');) {
    if (!line.empty()) {
      networks.push_back(line);
    }
  }

  std::sort(networks.begin(), networks.end());
  networks.erase(std::unique(networks.begin(), networks.end()), networks.end());
  return networks;
}

}  // namespace

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
    };
  }

  std::vector<std::string> names;
  if (const auto serialized = context.param("networks"); serialized.has_value()) {
    names = split_networks(*serialized);
  } else {
    std::array<std::string, 10> preview_names = {
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
    names.assign(preview_names.begin(), preview_names.end());
    std::sort(names.begin(), names.end());
  }

  if (names.empty()) {
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
    };
  }

  std::vector<components::ListItem> items;
  items.reserve(names.size());
  for (const auto& name : names) {
    items.push_back(components::ListItem{
        .id = "wifi-" + name,
        .primary_text = name,
        .intent = app::navigate_to("password", {{"ssid", name}}),
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
   };
}

}  // namespace rook::ui::screens
