#pragma once

#include "render/ScreenModel.hpp"
#include "screens/Screen.hpp"

namespace rook::ui::screens {

class DisconnectErrorScreen final : public Screen {
 public:
  [[nodiscard]] std::string_view id() const override;
  [[nodiscard]] std::string_view title() const override;
  [[nodiscard]] render::ScreenModel model(const ScreenContext& context) const override;
};

}  // namespace rook::ui::screens
