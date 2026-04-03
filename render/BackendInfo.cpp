#include "render/BackendInfo.hpp"

#include <sstream>

namespace rook::ui::render {

BackendInfo detect_backend_info() {
  BackendInfo info;

#ifdef ROOK_UI_HAS_SDL2
  info.has_sdl2 = true;
#endif

#ifdef ROOK_UI_HAS_RMLUI
  info.has_rmlui = true;
#endif

  info.preferred_preview_backend = info.has_sdl2 && info.has_rmlui ? "sdl2+rmlui" : "terminal-fallback";
  info.preferred_target_backend = info.has_sdl2 && info.has_rmlui ? "sdl2+rmlui+kmsdrm" : "unconfigured";
  return info;
}

std::string describe_backend_info(const BackendInfo& info) {
  std::ostringstream stream;
  stream << "backend-preview=" << info.preferred_preview_backend
         << " backend-target=" << info.preferred_target_backend
         << " sdl2=" << (info.has_sdl2 ? "yes" : "no")
         << " rmlui=" << (info.has_rmlui ? "yes" : "no");
  return stream.str();
}

}  // namespace rook::ui::render
