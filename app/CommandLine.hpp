#pragma once

#include <optional>
#include <string>

#include "app/AppConfig.hpp"

namespace rook::ui::app {

struct ParseResult {
  std::optional<AppConfig> config;
  std::optional<std::string> message;
  int exit_code = 0;
};

ParseResult parse_command_line(int argc, char** argv);

}  // namespace rook::ui::app
