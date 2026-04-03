#include "app/ScreenRegistry.hpp"

#include <algorithm>

#include "screens/PlaceholderScreen.hpp"

namespace rook::ui::app {

void ScreenRegistry::register_screen(std::string id, ScreenFactory factory) {
  factories_.insert_or_assign(std::move(id), std::move(factory));
}

bool ScreenRegistry::contains(std::string_view id) const {
  return factories_.contains(std::string(id));
}

std::unique_ptr<screens::Screen> ScreenRegistry::create(std::string_view id) const {
  const auto iterator = factories_.find(std::string(id));
  if (iterator == factories_.end()) {
    return nullptr;
  }

  return iterator->second();
}

std::vector<std::string> ScreenRegistry::ids() const {
  std::vector<std::string> result;
  result.reserve(factories_.size());

  for (const auto& [id, _] : factories_) {
    result.push_back(id);
  }

  std::sort(result.begin(), result.end());
  return result;
}

ScreenRegistry create_default_screen_registry() {
  ScreenRegistry registry;

  const auto register_placeholder = [&registry](std::string id, std::string title, std::vector<std::string> body, std::vector<std::string> actions) {
    const std::string registry_id = id;

    registry.register_screen(registry_id, [screen_id = std::move(id), screen_title = std::move(title), body_lines = std::move(body), action_lines = std::move(actions)] {
      components::ActionRow action_row{
          .id = screen_id + "-actions",
          .items = {},
      };

      for (std::size_t index = 0; index < action_lines.size(); ++index) {
        action_row.items.push_back(components::ActionItem{
            .id = screen_id + "-action-" + std::to_string(index),
            .label = action_lines[index],
            .intent = app::noop(),
        });
      }

      return std::make_unique<screens::PlaceholderScreen>(
          render::ScreenModel{
              .screen_id = screen_id,
              .title = screen_title,
              .body_lines = body_lines,
              .actions = std::move(action_row),
              .footer_hint = "Normalbetrieb-Stub aus Plan 01/02.",
          });
    });
  };

  register_placeholder("welcome", "RooK Service", {"Willkommensbildschirm ist verdrahtet."}, {"Weiter", "Beenden"});
  register_placeholder("status", "RooK Service", {"Service-PIN", "473 921"}, {"Trennen", "Beenden"});
  register_placeholder("wifi-list", "Mit WLAN verbinden", {"WLAN-Liste ist verdrahtet."}, {"Auswaehlen", "Zurueck"});
  register_placeholder("keyboard", "WLAN-Passwort eingeben", {"Keyboard-Screen ist verdrahtet."}, {"Verbinden", "Zurueck"});
  register_placeholder("wifi-wait", "Mit WLAN verbinden", {"Verbindung zum WLAN wird hergestellt"}, {});
  register_placeholder("vpn-wait", "Sichere Verbindung", {"Sichere Verbindung wird aufgebaut"}, {});
  register_placeholder("wifi-error", "WLAN-Verbindung fehlgeschlagen", {"Verbindung zum WLAN konnte nicht hergestellt werden"}, {"Zurueck zur WLAN-Liste", "Abbrechen und Beenden"});
  register_placeholder("vpn-error", "Verbindung fehlgeschlagen", {"Verbindung konnte nicht aufgebaut werden"}, {"WLAN wechseln", "Abbrechen und Beenden"});

  return registry;
}

}  // namespace rook::ui::app
