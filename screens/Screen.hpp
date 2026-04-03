#pragma once

#include <string>
#include <string_view>

#include "app/RuntimeMode.hpp"
#include "render/ScreenModel.hpp"

namespace rook::ui::screens {

class Screen {
 public:
  virtual ~Screen() = default;

  [[nodiscard]] virtual std::string_view id() const = 0;
  [[nodiscard]] virtual std::string_view title() const = 0;
  [[nodiscard]] virtual render::ScreenModel model(app::RuntimeMode mode) const = 0;
};

}  // namespace rook::ui::screens
