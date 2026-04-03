#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "app/NavigationSession.hpp"
#include "app/AppPaths.hpp"
#include "render/ScreenModel.hpp"

namespace rook::ui::app {

  class PreviewRegistry {
 public:
  void register_default_state(render::ScreenModel model);
  [[nodiscard]] bool contains(std::string_view screen_id) const;
  [[nodiscard]] render::ScreenModel model_for(const StartRequest& request) const;
  [[nodiscard]] std::vector<std::string> ids() const;

 private:
  std::unordered_map<std::string, render::ScreenModel> models_;
};

PreviewRegistry create_default_preview_registry(const AppPaths& paths);

}  // namespace rook::ui::app
