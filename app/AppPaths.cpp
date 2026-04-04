#include "app/AppPaths.hpp"

#include <cstdlib>
#include <string_view>

namespace rook::ui::app {

namespace {

constexpr std::string_view kPackagedResourceRoot = "/usr/share/rook-console-ui/resources";

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

std::filesystem::path detect_resource_root() {
  if (const char* override_root = std::getenv("ROOK_UI_RESOURCE_ROOT"); override_root != nullptr &&
      override_root[0] != '\0') {
    return std::filesystem::path(override_root);
  }

  for (auto current = std::filesystem::current_path(); !current.empty(); current = current.parent_path()) {
    if (std::filesystem::exists(current / "resources")) {
      return current / "resources";
    }

    if (current == current.root_path()) {
      break;
    }
  }

  const std::filesystem::path packaged_root(kPackagedResourceRoot);
  if (std::filesystem::exists(packaged_root)) {
    return packaged_root;
  }

  return std::filesystem::current_path() / "resources";
}

}  // namespace

AppPaths detect_app_paths() {
  const auto config_root = detect_config_root();
  const auto resource_root = detect_resource_root();

  return AppPaths{
      .config_root = config_root,
      .settings_file = config_root / "settings.json",
      .resource_root = resource_root,
  };
}

}  // namespace rook::ui::app
