#pragma once

#include <string>

namespace rook::ui::render {

struct BackendInfo {
  bool has_sdl2 = false;
  bool has_rmlui = false;
  std::string preferred_preview_backend;
  std::string preferred_target_backend;
};

BackendInfo detect_backend_info();
std::string describe_backend_info(const BackendInfo& info);

}  // namespace rook::ui::render
