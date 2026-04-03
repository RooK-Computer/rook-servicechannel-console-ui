#pragma once

#include "app/AppConfig.hpp"
#include "app/FocusEngine.hpp"
#include "app/NavigationSession.hpp"
#include "app/PreviewRegistry.hpp"
#include "app/ScreenRegistry.hpp"

namespace rook::ui::app {

class Application {
 public:
  explicit Application(AppConfig config);

  [[nodiscard]] int run() const;

 private:
  [[nodiscard]] const std::string& resolve_start_screen() const;
  [[nodiscard]] int run_preview() const;
  [[nodiscard]] int run_normal() const;
  [[nodiscard]] StartRequest create_start_request() const;

  AppConfig config_;
  PreviewRegistry preview_registry_;
  ScreenRegistry screen_registry_;
};

}  // namespace rook::ui::app
