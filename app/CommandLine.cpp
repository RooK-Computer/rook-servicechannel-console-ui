#include "app/CommandLine.hpp"

#include <string_view>

namespace rook::ui::app {

namespace {

ParseResult success(AppConfig config) {
  return ParseResult{
      .config = std::move(config),
      .message = std::nullopt,
      .exit_code = 0,
  };
}

ParseResult failure(std::string message, int exit_code) {
  return ParseResult{
      .config = std::nullopt,
      .message = std::move(message),
      .exit_code = exit_code,
  };
}

std::string usage_text() {
  return
      "Usage:\n"
      "  rook-ui\n"
      "  rook-ui --preview <screen-id>\n"
      "  rook-ui --help\n";
}

}  // namespace

ParseResult parse_command_line(int argc, char** argv) {
  AppConfig config;

  for (int index = 1; index < argc; ++index) {
    const std::string_view argument(argv[index]);

    if (argument == "--help" || argument == "-h") {
      return failure(usage_text(), 0);
    }

    if (argument == "--preview") {
      if (index + 1 >= argc) {
        return failure("Missing screen id after --preview.\n\n" + usage_text(), 2);
      }

      config.runtime_mode = RuntimeMode::Preview;
      config.preview_screen_id = argv[index + 1];
      ++index;
      continue;
    }

    return failure("Unknown argument: " + std::string(argument) + "\n\n" + usage_text(), 2);
  }

  return success(std::move(config));
}

}  // namespace rook::ui::app
