#include "app/FocusEngine.hpp"

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

}  // namespace

FocusEngine::FocusEngine(const render::ScreenModel& model) : model_(model) {}

FocusState FocusEngine::initial_state() const {
  if (model_.dialog.has_value() && !model_.dialog->actions.empty()) {
    return FocusState{.area = FocusArea::Dialog, .item_index = 0};
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

  if (state.area == FocusArea::ActionRow && state.item_index < model_.actions.items.size()) {
    return model_.actions.items[state.item_index].intent;
  }

  return std::nullopt;
}

}  // namespace rook::ui::app
