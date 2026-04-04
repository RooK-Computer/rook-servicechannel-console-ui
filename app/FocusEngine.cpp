#include "app/FocusEngine.hpp"

#include <algorithm>

namespace rook::ui::app {

namespace {

std::size_t clamp_index(std::size_t index, std::size_t count) {
  if (count == 0) {
    return 0;
  }

  if (index >= count) {
    return count - 1;
  }

  return index;
}

std::size_t keyboard_item_count(const components::KeyboardModel& keyboard) {
  std::size_t count = 0;
  for (const auto& row : keyboard.rows) {
    count += row.items.size();
  }
  return count;
}

struct KeyboardPosition {
  std::size_t row = 0;
  std::size_t column = 0;
};

KeyboardPosition keyboard_position_for_index(const components::KeyboardModel& keyboard, std::size_t flat_index) {
  std::size_t offset = 0;

  for (std::size_t row_index = 0; row_index < keyboard.rows.size(); ++row_index) {
    const auto& row = keyboard.rows[row_index];
    if (flat_index < offset + row.items.size()) {
      return KeyboardPosition{
          .row = row_index,
          .column = flat_index - offset,
      };
    }
    offset += row.items.size();
  }

  if (keyboard.rows.empty() || keyboard.rows.back().items.empty()) {
    return {};
  }

  return KeyboardPosition{
      .row = keyboard.rows.size() - 1,
      .column = keyboard.rows.back().items.size() - 1,
  };
}

std::size_t keyboard_index_for_position(
    const components::KeyboardModel& keyboard,
    std::size_t row_index,
    std::size_t column_index) {
  if (keyboard.rows.empty()) {
    return 0;
  }

  row_index = std::min(row_index, keyboard.rows.size() - 1);
  const auto& row = keyboard.rows[row_index];
  if (row.items.empty()) {
    return 0;
  }

  column_index = std::min(column_index, row.items.size() - 1);

  std::size_t offset = 0;
  for (std::size_t index = 0; index < row_index; ++index) {
    offset += keyboard.rows[index].items.size();
  }

  return offset + column_index;
}

std::size_t keyboard_index_for_id(const components::KeyboardModel& keyboard, std::string_view key_id) {
  std::size_t flat_index = 0;
  for (const auto& row : keyboard.rows) {
    for (const auto& item : row.items) {
      if (item.id == key_id) {
        return flat_index;
      }
      ++flat_index;
    }
  }

  return 0;
}

std::size_t keyboard_last_row_start(const components::KeyboardModel& keyboard) {
  if (keyboard.rows.empty()) {
    return 0;
  }

  std::size_t offset = 0;
  for (std::size_t index = 0; index + 1 < keyboard.rows.size(); ++index) {
    offset += keyboard.rows[index].items.size();
  }

  return offset;
}

}  // namespace

FocusEngine::FocusEngine(const render::ScreenModel& model) : model_(model) {}

FocusState FocusEngine::initial_state() const {
  if (model_.dialog.has_value() && !model_.dialog->actions.empty()) {
    return FocusState{.area = FocusArea::Dialog, .item_index = 0};
  }

  if (model_.keyboard.has_value() && keyboard_item_count(*model_.keyboard) > 0) {
    return FocusState{
        .area = FocusArea::Keyboard,
        .item_index = keyboard_index_for_id(*model_.keyboard, model_.keyboard->focus_key_id),
    };
  }

  if (!model_.list.items.empty()) {
    return FocusState{.area = FocusArea::List, .item_index = 0};
  }

  if (!model_.actions.items.empty()) {
    return FocusState{.area = FocusArea::ActionRow, .item_index = 0};
  }

  return FocusState{};
}

FocusState FocusEngine::move(FocusState state, InputCommand command) const {
  if (state.area == FocusArea::Dialog) {
    if (model_.dialog->actions.empty()) {
      return state;
    }

    if (command == InputCommand::Left && state.item_index > 0) {
      --state.item_index;
    } else if (command == InputCommand::Right && state.item_index + 1 < model_.dialog->actions.size()) {
      ++state.item_index;
    }

    return state;
  }

  if (state.area == FocusArea::Keyboard && model_.keyboard.has_value()) {
    const auto& keyboard = *model_.keyboard;
    if (keyboard_item_count(keyboard) == 0) {
      return state;
    }

    const auto position = keyboard_position_for_index(keyboard, state.item_index);
    const auto& row = keyboard.rows[position.row];

    if (command == InputCommand::Left && position.column > 0) {
      --state.item_index;
    } else if (command == InputCommand::Right && position.column + 1 < row.items.size()) {
      ++state.item_index;
    } else if (command == InputCommand::Up && position.row > 0) {
      state.item_index = keyboard_index_for_position(keyboard, position.row - 1, position.column);
    } else if (command == InputCommand::Down) {
      if (position.row + 1 < keyboard.rows.size()) {
        state.item_index = keyboard_index_for_position(keyboard, position.row + 1, position.column);
      } else if (!model_.actions.items.empty()) {
        state.area = FocusArea::ActionRow;
        state.item_index = 0;
      }
    }

    return state;
  }

  if (state.area == FocusArea::List) {
    if (command == InputCommand::Up && state.item_index > 0) {
      --state.item_index;
    } else if (command == InputCommand::Down) {
      if (state.item_index + 1 < model_.list.items.size()) {
        ++state.item_index;
      } else if (!model_.actions.items.empty()) {
        state.area = FocusArea::ActionRow;
        state.item_index = 0;
      }
    }

    return state;
  }

  if (state.area == FocusArea::ActionRow) {
    if (command == InputCommand::Left && state.item_index > 0) {
      --state.item_index;
    } else if (command == InputCommand::Right && state.item_index + 1 < model_.actions.items.size()) {
      ++state.item_index;
    } else if (command == InputCommand::Up && model_.keyboard.has_value() && keyboard_item_count(*model_.keyboard) > 0) {
      state.area = FocusArea::Keyboard;
      state.item_index = keyboard_last_row_start(*model_.keyboard);
    } else if (command == InputCommand::Up && !model_.list.items.empty()) {
      state.area = FocusArea::List;
      state.item_index = model_.list.items.size() - 1;
    } else if (command == InputCommand::Down) {
      state.item_index = clamp_index(state.item_index, model_.actions.items.size());
    }

    return state;
  }

  return state;
}

bool FocusEngine::is_focused_list_item(const FocusState& state, std::size_t index) const {
  return state.area == FocusArea::List && state.item_index == index;
}

bool FocusEngine::is_focused_action(const FocusState& state, std::size_t index) const {
  return state.area == FocusArea::ActionRow && state.item_index == index;
}

bool FocusEngine::is_focused_dialog_action(const FocusState& state, std::size_t index) const {
  return state.area == FocusArea::Dialog && state.item_index == index;
}

std::optional<Intent> FocusEngine::intent_for(const FocusState& state) const {
  if (state.area == FocusArea::Dialog && model_.dialog.has_value() && state.item_index < model_.dialog->actions.size()) {
    return model_.dialog->actions[state.item_index].intent;
  }

  if (state.area == FocusArea::List && state.item_index < model_.list.items.size()) {
    return model_.list.items[state.item_index].intent;
  }

  if (state.area == FocusArea::Keyboard && model_.keyboard.has_value()) {
    std::size_t flat_index = 0;
    for (const auto& row : model_.keyboard->rows) {
      for (const auto& item : row.items) {
        if (flat_index == state.item_index) {
          return item.intent;
        }
        ++flat_index;
      }
    }
  }

  if (state.area == FocusArea::ActionRow && state.item_index < model_.actions.items.size()) {
    return model_.actions.items[state.item_index].intent;
  }

  return std::nullopt;
}

}  // namespace rook::ui::app
