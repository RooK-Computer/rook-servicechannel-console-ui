#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "app/Intent.hpp"

namespace rook::ui::components {

struct ActionItem {
  std::string id;
  std::string label;
  app::Intent intent = app::noop();
};

struct ActionRow {
  std::string id;
  std::vector<ActionItem> items;
};

struct GridItem {
  std::string id;
  std::string label;
  app::Intent intent = app::noop();
  std::size_t width = 1;
  bool active = false;
};

struct GridRow {
  std::string id;
  std::vector<GridItem> items;
};

struct KeyboardModel {
  std::string id;
  std::string ssid;
  std::string password;
  std::string helper_text;
  std::string layout_label;
  std::string focus_key_id;
  std::vector<GridRow> rows;
};

struct ListItem {
  std::string id;
  std::string primary_text;
  std::string secondary_text;
  app::Intent intent = app::noop();
};

struct ListSection {
  std::string id;
  std::string title;
  std::vector<ListItem> items;
  std::optional<std::string> empty_state_text;
};

struct DialogModel {
  std::string id;
  std::string title;
  std::vector<std::string> body_lines;
  std::vector<ActionItem> actions;
};

}  // namespace rook::ui::components
