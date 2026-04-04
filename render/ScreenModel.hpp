#pragma once

#include <optional>
#include <string>
#include <vector>

#include "components/UiModels.hpp"

namespace rook::ui::render {

struct ScreenModel {
  std::string screen_id;
  std::string title;
  std::vector<std::string> body_lines;
  components::ListSection list;
  components::ActionRow actions;
  std::optional<components::DialogModel> dialog;
  bool show_spinner = false;
  bool allow_back = true;
  std::string footer_hint;
};

}  // namespace rook::ui::render
