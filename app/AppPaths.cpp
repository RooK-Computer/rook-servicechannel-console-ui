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

std::filesystem::path detect_project_root() {
  for (auto current = std::filesystem::current_path(); !current.empty(); current = current.parent_path()) {
    if (std::filesystem::exists(current / "resources") &&
        std::filesystem::exists(current / "third_party" / "rmlui")) {
      return current;
    }

    if (current == current.root_path()) {
      break;
    }
  }

  return std::filesystem::current_path();
}

}  // namespace

AppPaths detect_app_paths() {
  const auto config_root = detect_config_root();
  const auto project_root = detect_project_root();

  return AppPaths{
      .config_root = config_root,
      .settings_file = config_root / "settings.json",
      .resource_root = project_root / "resources",
  };
}

}  // namespace rook::ui::app
