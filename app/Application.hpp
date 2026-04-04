#pragma once

#include <ostream>

#include "app/AppConfig.hpp"
#include "app/FocusEngine.hpp"
#include "app/NavigationSession.hpp"
#include "app/PreviewRegistry.hpp"
#include "app/ScreenRegistry.hpp"
#include "render/ScreenModel.hpp"

namespace rook::ui::app {

class Application {
 public:
  explicit Application(AppConfig config);

  [[nodiscard]] int run() const;
  int print_screen_list(std::ostream& out) const;

 private:
  [[nodiscard]] const std::string& resolve_start_screen() const;
  [[nodiscard]] bool is_known_screen_id(std::string_view screen_id) const;
  [[nodiscard]] render::ScreenModel resolve_model(const StartRequest& request) const;
  [[nodiscard]] int run_graphical() const;
  [[nodiscard]] int run_preview() const;
  [[nodiscard]] int run_normal() const;
  [[nodiscard]] StartRequest create_start_request() const;

  AppConfig config_;
  PreviewRegistry preview_registry_;
  ScreenRegistry screen_registry_;
};

}  // namespace rook::ui::app
