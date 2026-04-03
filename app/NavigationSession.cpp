#include "app/NavigationSession.hpp"

namespace rook::ui::app {

NavigationSession::NavigationSession(StartRequest start_request) : current_(std::move(start_request)) {}

const StartRequest& NavigationSession::current() const { return current_; }

bool NavigationSession::can_go_back() const { return !stack_.empty(); }

bool NavigationSession::go_back() {
  if (stack_.empty()) {
    return false;
  }

  current_ = stack_.back();
  stack_.pop_back();
  return true;
}

bool NavigationSession::apply(const Intent& intent) {
  switch (intent.kind) {
    case IntentKind::NavigateTo:
      stack_.push_back(current_);
      current_ = StartRequest{
          .screen_id = intent.target_screen_id,
          .params = intent.params,
      };
      return true;
    case IntentKind::GoBack:
      return go_back();
    case IntentKind::CloseApp:
      return false;
    case IntentKind::NoOp:
      return true;
  }

  return true;
}

}  // namespace rook::ui::app
