#include "app/Intent.hpp"

namespace rook::ui::app {

Intent navigate_to(std::string target_screen_id, IntentParams params) {
  return Intent{
      .kind = IntentKind::NavigateTo,
      .target_screen_id = std::move(target_screen_id),
      .params = std::move(params),
  };
}

Intent go_back() {
  return Intent{
      .kind = IntentKind::GoBack,
  };
}

Intent close_app() {
  return Intent{
      .kind = IntentKind::CloseApp,
  };
}

Intent noop() {
  return Intent{
      .kind = IntentKind::NoOp,
  };
}

}  // namespace rook::ui::app
