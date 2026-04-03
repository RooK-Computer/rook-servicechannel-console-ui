#pragma once

#include <string>
#include <vector>

#include "app/Intent.hpp"

namespace rook::ui::app {

struct StartRequest {
  std::string screen_id;
  IntentParams params;
};

class NavigationSession {
 public:
  explicit NavigationSession(StartRequest start_request);

  [[nodiscard]] const StartRequest& current() const;
  [[nodiscard]] bool can_go_back() const;
  bool go_back();
  bool apply(const Intent& intent);

 private:
  StartRequest current_;
  std::vector<StartRequest> stack_;
};

}  // namespace rook::ui::app
