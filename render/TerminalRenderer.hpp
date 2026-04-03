#pragma once

#include <optional>

#include "app/FocusEngine.hpp"
#include "render/BackendInfo.hpp"
#include "render/ScreenModel.hpp"
#include "theme/Theme.hpp"

namespace rook::ui::render {

class TerminalRenderer {
 public:
  void render_screen(
      const ScreenModel& model,
      const theme::Theme& theme,
      const BackendInfo& backend_info,
      std::optional<app::FocusState> focus_state) const;
};

}  // namespace rook::ui::render
