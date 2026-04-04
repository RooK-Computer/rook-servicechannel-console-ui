#include "app/PreviewRegistry.hpp"

#include <algorithm>
#include <stdexcept>

namespace rook::ui::app {

void PreviewRegistry::register_preview_screen(std::string screen_id, IntentParams params) {
  defaults_.insert_or_assign(std::move(screen_id), std::move(params));
}

bool PreviewRegistry::contains(std::string_view screen_id) const {
  return defaults_.contains(std::string(screen_id));
}

StartRequest PreviewRegistry::start_request_for(std::string_view screen_id) const {
  const auto iterator = defaults_.find(std::string(screen_id));
  if (iterator == defaults_.end()) {
    throw std::runtime_error("unknown preview screen");
  }

  return StartRequest{
      .screen_id = std::string(screen_id),
      .params = iterator->second,
  };
}

std::vector<std::string> PreviewRegistry::ids() const {
  std::vector<std::string> ids;
  ids.reserve(defaults_.size());

  for (const auto& [id, _] : defaults_) {
    ids.push_back(id);
  }

  std::sort(ids.begin(), ids.end());
  return ids;
}

PreviewRegistry create_default_preview_registry(const AppPaths& paths) {
  (void)paths;
  PreviewRegistry registry;
  registry.register_preview_screen("welcome");
  registry.register_preview_screen("status");
  registry.register_preview_screen("wifi-list");
  registry.register_preview_screen("keyboard", {{"ssid", "RooK-Setup"}});
  registry.register_preview_screen("wifi-wait");
  registry.register_preview_screen("vpn-wait");
  registry.register_preview_screen("wifi-error");
  registry.register_preview_screen("vpn-error");

  return registry;
}

}  // namespace rook::ui::app
