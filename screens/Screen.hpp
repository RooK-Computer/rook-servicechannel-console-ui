#pragma once

#include <optional>
#include <string>
#include <string_view>

#include "app/Intent.hpp"
#include "app/RuntimeMode.hpp"
#include "render/ScreenModel.hpp"

namespace rook::ui::screens {

struct ScreenContext {
  app::RuntimeMode runtime_mode = app::RuntimeMode::Normal;
  app::IntentParams params;

  [[nodiscard]] bool is_preview() const { return runtime_mode == app::RuntimeMode::Preview; }

  [[nodiscard]] std::optional<std::string_view> param(std::string_view key) const {
    if (const auto iterator = params.find(std::string(key)); iterator != params.end()) {
      return iterator->second;
    }

    return std::nullopt;
  }

  [[nodiscard]] std::string param_or(std::string_view key, std::string fallback) const {
    if (const auto value = param(key); value.has_value()) {
      return std::string(*value);
    }

    return fallback;
  }

  [[nodiscard]] bool flag(std::string_view key) const {
    if (const auto value = param(key); value.has_value()) {
      return *value == "true" || *value == "1" || *value == "yes";
    }

    return false;
  }
};

class Screen {
 public:
  virtual ~Screen() = default;

  [[nodiscard]] virtual std::string_view id() const = 0;
  [[nodiscard]] virtual std::string_view title() const = 0;
  [[nodiscard]] virtual render::ScreenModel model(const ScreenContext& context) const = 0;
};

}  // namespace rook::ui::screens
