#include "screens/PlaceholderScreen.hpp"

namespace rook::ui::screens {

PlaceholderScreen::PlaceholderScreen(render::ScreenModel model) : model_(std::move(model)) {}

std::string_view PlaceholderScreen::id() const { return model_.screen_id; }

std::string_view PlaceholderScreen::title() const { return model_.title; }

render::ScreenModel PlaceholderScreen::model(const ScreenContext& context) const {
  auto result = model_;
  if (context.is_preview()) {
    result.footer_hint = "Preview ueber Platzhalter-Screen. Escape oder q beendet die Anzeige.";
  }
  return result;
}

}  // namespace rook::ui::screens
