#include "app/PreviewRegistry.hpp"

#include <algorithm>
#include <stdexcept>

#include "app/TextResources.hpp"

namespace rook::ui::app {

void PreviewRegistry::register_default_state(render::ScreenModel model) {
  models_.insert_or_assign(model.screen_id, std::move(model));
}

bool PreviewRegistry::contains(std::string_view screen_id) const {
  return models_.contains(std::string(screen_id));
}

render::ScreenModel PreviewRegistry::model_for(const StartRequest& request) const {
  const auto iterator = models_.find(std::string(request.screen_id));
  if (iterator == models_.end()) {
    throw std::runtime_error("unknown preview screen");
  }

  auto model = iterator->second;

  if (const auto ssid = request.params.find("ssid"); ssid != request.params.end()) {
    if (!model.body_lines.empty()) {
      model.body_lines[0] = "SSID: " + ssid->second;
    }
  }

  if (const auto dialog = request.params.find("dialog"); dialog != request.params.end()) {
    if (dialog->second == "disconnect") {
      model.dialog = components::DialogModel{
          .id = "disconnect-dialog",
          .title = "Service-Verbindung trennen",
          .body_lines = {"Service-Verbindung wirklich trennen?"},
          .actions = {
              components::ActionItem{.id = "disconnect-confirm", .label = "Trennen", .intent = app::navigate_to("vpn-error")},
              components::ActionItem{.id = "disconnect-back", .label = "Zurueck", .intent = app::go_back()},
          },
      };
    }
  } else {
    model.dialog.reset();
  }

  const bool hide_welcome = request.params.contains("hide-welcome");
  if (model.screen_id == "welcome" && !model.actions.items.empty()) {
    model.actions.items[0].label = hide_welcome
                                       ? "[x] Beim naechsten Start nicht mehr anzeigen"
                                       : "[ ] Beim naechsten Start nicht mehr anzeigen";
    model.actions.items[0].intent = hide_welcome
                                        ? app::navigate_to("welcome")
                                        : app::navigate_to("welcome", {{"hide-welcome", "true"}});
  }

  return model;
}

std::vector<std::string> PreviewRegistry::ids() const {
  std::vector<std::string> ids;
  ids.reserve(models_.size());

  for (const auto& [id, _] : models_) {
    ids.push_back(id);
  }

  std::sort(ids.begin(), ids.end());
  return ids;
}

PreviewRegistry create_default_preview_registry(const AppPaths& paths) {
  PreviewRegistry registry;
  const auto welcome_lines = load_welcome_text_lines(paths.resource_root);

  std::vector<components::ListItem> welcome_items;
  welcome_items.reserve(welcome_lines.size());
  for (std::size_t index = 0; index < welcome_lines.size(); ++index) {
    welcome_items.push_back(components::ListItem{
        .id = "welcome-text-" + std::to_string(index),
        .primary_text = welcome_lines[index],
    });
  }

  registry.register_default_state({
      .screen_id = "welcome",
      .title = "RooK Service",
      .list = components::ListSection{
          .id = "welcome-body",
          .title = "Einfuehrung",
          .items = std::move(welcome_items),
      },
      .actions = components::ActionRow{
          .id = "welcome-actions",
          .items = {
              components::ActionItem{.id = "welcome-hide", .label = "[ ] Beim naechsten Start nicht mehr anzeigen", .intent = app::navigate_to("welcome", {{"hide-welcome", "true"}})},
              components::ActionItem{.id = "welcome-continue", .label = "Weiter", .intent = app::navigate_to("wifi-list")},
              components::ActionItem{.id = "welcome-exit", .label = "Beenden", .intent = app::close_app()},
          },
      },
      .footer_hint = "Mit Stick oder Schultertasten scrollen",
  });

  registry.register_default_state({
      .screen_id = "status",
      .title = "RooK Service",
      .body_lines = {"Service-PIN", "473 921"},
      .actions = components::ActionRow{
          .id = "status-actions",
          .items = {
              components::ActionItem{.id = "status-disconnect", .label = "Trennen", .intent = app::navigate_to("status", {{"dialog", "disconnect"}})},
              components::ActionItem{.id = "status-exit", .label = "Beenden", .intent = app::close_app()},
          },
      },
      .footer_hint = "Aktiver Statusbildschirm mit grosser Service-PIN.",
  });

  registry.register_default_state({
      .screen_id = "wifi-list",
      .title = "Mit WLAN verbinden",
      .list = components::ListSection{
          .id = "wifi-list",
          .title = "Verfuegbare WLAN-Netze",
          .items = {
              components::ListItem{.id = "wifi-guest", .primary_text = "Gastnetz", .intent = app::navigate_to("keyboard", {{"ssid", "Gastnetz"}})},
              components::ListItem{.id = "wifi-rook", .primary_text = "RooK-Setup", .intent = app::navigate_to("keyboard", {{"ssid", "RooK-Setup"}})},
              components::ListItem{.id = "wifi-livingroom", .primary_text = "Wohnzimmer-5G", .intent = app::navigate_to("keyboard", {{"ssid", "Wohnzimmer-5G"}})},
          },
      },
      .actions = components::ActionRow{
          .id = "wifi-actions",
          .items = {
              components::ActionItem{.id = "wifi-back", .label = "Zurueck", .intent = app::go_back()},
          },
      },
      .footer_hint = "WLAN-Liste mit laufendem Scan im Default-Zustand.",
  });

  registry.register_default_state({
      .screen_id = "keyboard",
      .title = "WLAN-Passwort eingeben",
      .body_lines = {"SSID: RooK-Setup", "Passwort", "hunter2", "B loescht Zeichen, bei leerer Eingabe geht es zurueck."},
      .actions = components::ActionRow{
          .id = "keyboard-actions",
          .items = {
              components::ActionItem{.id = "keyboard-connect", .label = "Verbinden", .intent = app::navigate_to("wifi-wait")},
              components::ActionItem{.id = "keyboard-back", .label = "Zurueck", .intent = app::go_back()},
          },
      },
      .footer_hint = "QWERTZ-On-Screen-Keyboard im Produktstil folgt im Render-Ausbau.",
  });

  registry.register_default_state({
      .screen_id = "wifi-wait",
      .title = "Mit WLAN verbinden",
      .body_lines = {"Verbindung zum WLAN wird hergestellt", "Animationen folgen mit dem Render-Backend."},
      .actions = components::ActionRow{
          .id = "wifi-wait-actions",
          .items = {
              components::ActionItem{.id = "wifi-wait-next", .label = "Weiter zu VPN", .intent = app::navigate_to("vpn-wait")},
          },
      },
      .footer_hint = "WLAN-Warte-Screen.",
  });

  registry.register_default_state({
      .screen_id = "vpn-wait",
      .title = "Sichere Verbindung",
      .body_lines = {"Sichere Verbindung wird aufgebaut", "Animationen folgen mit dem Render-Backend."},
      .actions = components::ActionRow{
          .id = "vpn-wait-actions",
          .items = {
              components::ActionItem{.id = "vpn-wait-ok", .label = "Zum Status", .intent = app::navigate_to("status")},
              components::ActionItem{.id = "vpn-wait-error", .label = "Fehlerzustand", .intent = app::navigate_to("vpn-error")},
          },
      },
      .footer_hint = "VPN-Warte-Screen.",
  });

  registry.register_default_state({
      .screen_id = "wifi-error",
      .title = "WLAN-Verbindung fehlgeschlagen",
      .body_lines = {"Verbindung zum WLAN konnte nicht hergestellt werden"},
      .actions = components::ActionRow{
          .id = "wifi-error-actions",
          .items = {
              components::ActionItem{.id = "wifi-error-back", .label = "Zurueck zur WLAN-Liste", .intent = app::navigate_to("wifi-list")},
              components::ActionItem{.id = "wifi-error-exit", .label = "Abbrechen und Beenden", .intent = app::close_app()},
          },
      },
      .footer_hint = "WLAN-Fehler-Screen.",
  });

  registry.register_default_state({
      .screen_id = "vpn-error",
      .title = "Verbindung fehlgeschlagen",
      .body_lines = {"Verbindung konnte nicht aufgebaut werden"},
      .actions = components::ActionRow{
          .id = "vpn-error-actions",
          .items = {
              components::ActionItem{.id = "vpn-error-retry", .label = "WLAN wechseln", .intent = app::navigate_to("wifi-list")},
              components::ActionItem{.id = "vpn-error-exit", .label = "Abbrechen und Beenden", .intent = app::close_app()},
          },
      },
      .footer_hint = "VPN-Fehler-Screen.",
  });

  return registry;
}

}  // namespace rook::ui::app
