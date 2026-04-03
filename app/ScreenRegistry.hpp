#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "screens/Screen.hpp"

namespace rook::ui::app {

class ScreenRegistry {
 public:
  using ScreenFactory = std::function<std::unique_ptr<screens::Screen>()>;

  void register_screen(std::string id, ScreenFactory factory);
  [[nodiscard]] bool contains(std::string_view id) const;
  [[nodiscard]] std::unique_ptr<screens::Screen> create(std::string_view id) const;
  [[nodiscard]] std::vector<std::string> ids() const;

 private:
  std::unordered_map<std::string, ScreenFactory> factories_;
};

ScreenRegistry create_default_screen_registry();

}  // namespace rook::ui::app
