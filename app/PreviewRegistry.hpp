#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

#include "app/AppPaths.hpp"
#include "app/NavigationSession.hpp"

namespace rook::ui::app {

class PreviewRegistry {
 public:
  void register_preview_screen(std::string screen_id, IntentParams params = {});
  [[nodiscard]] bool contains(std::string_view screen_id) const;
  [[nodiscard]] StartRequest start_request_for(std::string_view screen_id) const;
  [[nodiscard]] std::vector<std::string> ids() const;

 private:
  std::unordered_map<std::string, IntentParams> defaults_;
};

PreviewRegistry create_default_preview_registry(const AppPaths& paths);

}  // namespace rook::ui::app
