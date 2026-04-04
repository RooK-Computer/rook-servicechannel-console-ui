#include "render/TerminalRenderer.hpp"

#include <iostream>
#include <optional>
#include <string>

#include "theme/Theme.hpp"

namespace rook::ui::render {

namespace {

void clear_screen() {
  std::cout << "\033[2J\033[H";
}

void print_divider() {
  std::cout << "+--------------------------------------------------+\n";
}

void print_line(const std::string& text) {
  constexpr std::size_t content_width = 50;
  std::string clipped = text.substr(0, content_width);
  if (clipped.size() < content_width) {
    clipped.append(content_width - clipped.size(), ' ');
  }

  std::cout << "| " << clipped << " |\n";
}

std::string focus_prefix(bool focused) {
  return focused ? "> " : "  ";
}

void print_keyboard(
    const components::KeyboardModel& keyboard,
    const std::optional<app::FocusState>& focus_state) {
  print_line("SSID: " + keyboard.ssid);
  print_line("Passwort:");
  print_line(keyboard.password.empty() ? "<leer>" : keyboard.password);
  print_line(keyboard.layout_label);
  print_line(keyboard.helper_text);
  print_divider();

  std::size_t flat_index = 0;
  for (const auto& row : keyboard.rows) {
    std::string line;
    for (const auto& item : row.items) {
      const bool focused = focus_state.has_value() &&
                           focus_state->area == app::FocusArea::Keyboard &&
                           focus_state->item_index == flat_index;
      if (!line.empty()) {
        line += ' ';
      }
      line += focused ? "[" : " ";
      line += item.label;
      if (item.active) {
        line += "*";
      }
      line += focused ? "]" : " ";
      ++flat_index;
    }
    print_line(line);
  }
}

void print_list(
    const ScreenModel& model,
    const std::optional<app::FocusState>& focus_state) {
  if (!model.list.title.empty()) {
    print_line(model.list.title);
  }

  if (model.list.items.empty()) {
    if (model.list.empty_state_text.has_value()) {
      print_line(*model.list.empty_state_text);
    }
    return;
  }

  for (std::size_t index = 0; index < model.list.items.size(); ++index) {
    const bool focused = focus_state.has_value() &&
                         focus_state->area == app::FocusArea::List &&
                         focus_state->item_index == index;
    print_line(focus_prefix(focused) + model.list.items[index].primary_text);
  }
}

void print_actions(
    const std::vector<components::ActionItem>& actions,
    app::FocusArea area,
    const std::optional<app::FocusState>& focus_state) {
  for (std::size_t index = 0; index < actions.size(); ++index) {
    const bool focused = focus_state.has_value() &&
                         focus_state->area == area &&
                         focus_state->item_index == index;
    print_line(focus_prefix(focused) + actions[index].label);
  }
}

}  // namespace

void TerminalRenderer::render_screen(
    const ScreenModel& model,
    const theme::Theme& theme,
    const BackendInfo& backend_info,
    std::optional<app::FocusState> focus_state) const {
  clear_screen();
  print_divider();
  print_line(model.title);
  print_divider();

  for (const auto& line : model.body_lines) {
    print_line(line);
  }

  if (!model.body_lines.empty()) {
    print_divider();
  }

  if (model.keyboard.has_value()) {
    print_keyboard(*model.keyboard, focus_state);
    print_divider();
  }

  print_list(model, focus_state);
  if (!model.list.items.empty() || model.list.empty_state_text.has_value()) {
    print_divider();
  }

  print_actions(model.actions.items, app::FocusArea::ActionRow, focus_state);
  print_divider();

  if (model.dialog.has_value()) {
    print_line("Dialog");
    print_divider();
    print_line(model.dialog->title);
    for (const auto& line : model.dialog->body_lines) {
      print_line(line);
    }
    print_divider();
    print_actions(model.dialog->actions, app::FocusArea::Dialog, focus_state);
    print_divider();
  }

  std::cout << "Steuerung: Pfeile oder WASD, Enter/Space bestaetigt, B geht zurueck, Escape/Q beendet.\n";
}

}  // namespace rook::ui::render
