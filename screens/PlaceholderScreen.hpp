#pragma once

#include <string>
#include <string_view>

#include "render/ScreenModel.hpp"
#include "screens/Screen.hpp"

namespace rook::ui::screens {

class PlaceholderScreen final : public Screen {
 public:
  explicit PlaceholderScreen(render::ScreenModel model);

  [[nodiscard]] std::string_view id() const override;
  [[nodiscard]] std::string_view title() const override;
  [[nodiscard]] render::ScreenModel model(app::RuntimeMode mode) const override;

 private:
  render::ScreenModel model_;
};

}  // namespace rook::ui::screens
