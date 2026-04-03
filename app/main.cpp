#include <iostream>

#include "app/Application.hpp"
#include "app/CommandLine.hpp"

int main(int argc, char** argv) {
  const auto parse_result = rook::ui::app::parse_command_line(argc, argv);

  if (!parse_result.config.has_value()) {
    if (parse_result.message.has_value()) {
      if (parse_result.exit_code == 0) {
        std::cout << *parse_result.message;
      } else {
        std::cerr << *parse_result.message;
      }
    }

    return parse_result.exit_code;
  }

  const rook::ui::app::Application app(*parse_result.config);
  return app.run();
}
