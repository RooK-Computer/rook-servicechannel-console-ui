#include "app/RuntimeState.hpp"

namespace rook::ui::app {

bool has_connected_wifi(const RuntimeState& state) {
  return state.any_wifi_active || state.support_wifi_active || state.wifi_state == ConnectionState::Connected;
}

bool has_active_support_session(const RuntimeState& state) {
  return state.support_active || state.support_state == SupportState::OnlineVpnUp ||
         state.support_state == SupportState::ServiceMode;
}

}  // namespace rook::ui::app
