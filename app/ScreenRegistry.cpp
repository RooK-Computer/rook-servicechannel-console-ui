#include "app/ScreenRegistry.hpp"

#include <algorithm>

#include "screens/KeyboardScreen.hpp"
#include "screens/PlaceholderScreen.hpp"
#include "screens/StatusScreen.hpp"
#include "screens/VpnErrorScreen.hpp"
#include "screens/VpnWaitScreen.hpp"
#include "screens/WelcomeScreen.hpp"
#include "screens/WifiErrorScreen.hpp"
#include "screens/WifiListScreen.hpp"
#include "screens/WifiWaitScreen.hpp"

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

ScreenRegistry create_default_screen_registry(const AppPaths& paths) {
  ScreenRegistry registry;
  registry.register_screen("welcome", [paths] { return std::make_unique<screens::WelcomeScreen>(paths); });
  registry.register_screen("status", [] { return std::make_unique<screens::StatusScreen>(); });
  registry.register_screen("wifi-list", [] { return std::make_unique<screens::WifiListScreen>(); });
  registry.register_screen("keyboard", [] { return std::make_unique<screens::KeyboardScreen>(); });
  registry.register_screen("password", [] { return std::make_unique<screens::KeyboardScreen>(); });
  registry.register_screen("wifi-wait", [] { return std::make_unique<screens::WifiWaitScreen>(); });
  registry.register_screen("vpn-wait", [] { return std::make_unique<screens::VpnWaitScreen>(); });
  registry.register_screen("wifi-error", [] { return std::make_unique<screens::WifiErrorScreen>(); });
  registry.register_screen("vpn-error", [] { return std::make_unique<screens::VpnErrorScreen>(); });

  return registry;
}

}  // namespace rook::ui::app
