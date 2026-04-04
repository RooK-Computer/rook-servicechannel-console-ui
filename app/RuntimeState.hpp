#pragma once

#include <optional>
#include <string>
#include <vector>

namespace rook::ui::app {

enum class ConnectionState {
  Disconnected,
  Connected,
};

enum class SupportState {
  Idle,
  Online,
  OnlineVpnUp,
  ServiceMode,
};

struct RuntimeError {
  int code = 0;
  std::string message;
};

struct RuntimeState {
  bool support_active = false;
  SupportState support_state = SupportState::Idle;
  ConnectionState wifi_state = ConnectionState::Disconnected;
  ConnectionState vpn_state = ConnectionState::Disconnected;
  bool any_wifi_active = false;
  bool support_wifi_active = false;
  std::optional<std::string> active_wifi_connection;
  std::optional<std::string> pin;
  std::vector<std::string> wifi_networks;
  std::optional<RuntimeError> last_error;
};

[[nodiscard]] bool has_connected_wifi(const RuntimeState& state);
[[nodiscard]] bool has_active_support_session(const RuntimeState& state);

}  // namespace rook::ui::app
