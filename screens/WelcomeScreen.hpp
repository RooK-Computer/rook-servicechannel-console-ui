#pragma once

#include "app/AppPaths.hpp"
#include "render/ScreenModel.hpp"
#include "screens/Screen.hpp"

namespace rook::ui::screens {

class WelcomeScreen final : public Screen {
 public:
  explicit WelcomeScreen(app::AppPaths paths);

  [[nodiscard]] std::string_view id() const override;
  [[nodiscard]] std::string_view title() const override;
  [[nodiscard]] render::ScreenModel model(const ScreenContext& context) const override;

 private:
  app::AppPaths paths_;
};

}  // namespace rook::ui::screens
