#pragma once

#include <string>
#include <unordered_map>

namespace rook::ui::app {

using IntentParams = std::unordered_map<std::string, std::string>;

enum class IntentKind {
  NavigateTo,
  GoBack,
  CloseApp,
  NoOp,
};

struct Intent {
  IntentKind kind = IntentKind::NoOp;
  std::string target_screen_id;
  IntentParams params;
};

Intent navigate_to(std::string target_screen_id, IntentParams params = {});
Intent go_back();
Intent close_app();
Intent noop();

}  // namespace rook::ui::app
