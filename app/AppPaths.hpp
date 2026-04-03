#pragma once

#include <filesystem>

namespace rook::ui::app {

struct AppPaths {
  std::filesystem::path config_root;
  std::filesystem::path settings_file;
  std::filesystem::path resource_root;
};

AppPaths detect_app_paths();

}  // namespace rook::ui::app
