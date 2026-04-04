#pragma once

#include <cstddef>
#include <optional>

#include "app/InputCommand.hpp"
#include "render/ScreenModel.hpp"

namespace rook::ui::app {

enum class FocusArea {
  None,
  Keyboard,
  List,
  ActionRow,
  Dialog,
};

struct FocusState {
  FocusArea area = FocusArea::None;
  std::size_t item_index = 0;
};

class FocusEngine {
 public:
  explicit FocusEngine(const render::ScreenModel& model);

  [[nodiscard]] FocusState initial_state() const;
  [[nodiscard]] FocusState move(FocusState state, InputCommand command) const;
  [[nodiscard]] bool is_focused_list_item(const FocusState& state, std::size_t index) const;
  [[nodiscard]] bool is_focused_action(const FocusState& state, std::size_t index) const;
  [[nodiscard]] bool is_focused_dialog_action(const FocusState& state, std::size_t index) const;
  [[nodiscard]] std::optional<Intent> intent_for(const FocusState& state) const;

 private:
  const render::ScreenModel& model_;
};

}  // namespace rook::ui::app
