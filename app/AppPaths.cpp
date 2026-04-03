#include "app/AppPaths.hpp"

#include <cstdlib>

namespace rook::ui::app {

namespace {

std::filesystem::path detect_home_directory() {
  if (const char* home = std::getenv("HOME"); home != nullptr && home[0] != '\0') {
    return std::filesystem::path(home);
  }

  return std::filesystem::current_path();
}

std::filesystem::path detect_config_root() {
  if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg != nullptr && xdg[0] != '\0') {
    return std::filesystem::path(xdg) / "rook-ui";
  }

  return detect_home_directory() / ".config" / "rook-ui";
}

}  // namespace

AppPaths detect_app_paths() {
  const auto config_root = detect_config_root();

  return AppPaths{
      .config_root = config_root,
      .settings_file = config_root / "settings.json",
      .resource_root = std::filesystem::current_path() / "resources",
  };
}

}  // namespace rook::ui::app
