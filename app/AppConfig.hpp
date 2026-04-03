#pragma once

#include <string>

#include "app/AppPaths.hpp"
#include "app/RuntimeMode.hpp"

namespace rook::ui::app {

struct AppConfig {
  RuntimeMode runtime_mode = RuntimeMode::Normal;
  std::string initial_screen_id = "welcome";
  std::string preview_screen_id;
  AppPaths paths = detect_app_paths();
};

}  // namespace rook::ui::app
