#pragma once

#include <chrono>
#include <optional>

#include "app/InputCommand.hpp"

namespace rook::ui::app {

class TerminalInput {
 public:
  [[nodiscard]] InputCommand read_command(
      std::optional<std::chrono::milliseconds> timeout = std::nullopt) const;
};

}  // namespace rook::ui::app
