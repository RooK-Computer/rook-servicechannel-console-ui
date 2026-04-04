#pragma once

#include <memory>
#include <optional>

#include "app/AppPaths.hpp"
#include "app/FocusEngine.hpp"
#include "app/InputCommand.hpp"
#include "app/RuntimeMode.hpp"
#include "render/BackendInfo.hpp"
#include "render/ScreenModel.hpp"
#include "theme/Theme.hpp"

namespace rook::ui::render {

class RmlUiRenderer {
 public:
  RmlUiRenderer(app::AppPaths paths, app::RuntimeMode runtime_mode);
  ~RmlUiRenderer();

  RmlUiRenderer(const RmlUiRenderer&) = delete;
  RmlUiRenderer& operator=(const RmlUiRenderer&) = delete;
  RmlUiRenderer(RmlUiRenderer&&) noexcept;
  RmlUiRenderer& operator=(RmlUiRenderer&&) noexcept;

  [[nodiscard]] bool initialize();
  void render_screen(
      const ScreenModel& model,
      const theme::Theme& theme,
      const BackendInfo& backend_info,
      std::optional<app::FocusState> focus_state);
  [[nodiscard]] bool capture_screenshot(const std::string& path);
  [[nodiscard]] app::InputCommand read_command(bool animate);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace rook::ui::render
