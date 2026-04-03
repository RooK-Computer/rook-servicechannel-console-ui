#pragma once

#include "app/InputCommand.hpp"

namespace rook::ui::app {

class TerminalInput {
 public:
  [[nodiscard]] InputCommand read_command() const;
};

}  // namespace rook::ui::app
