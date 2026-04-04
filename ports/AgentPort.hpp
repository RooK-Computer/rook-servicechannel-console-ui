#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "app/RuntimeState.hpp"

namespace rook::ui::ports {

struct WifiScanCompletedEvent {
  std::vector<std::string> networks;
};

struct WifiConnectionStateChangedEvent {
  app::ConnectionState state = app::ConnectionState::Disconnected;
};

struct VpnStateChangedEvent {
  app::ConnectionState state = app::ConnectionState::Disconnected;
};

struct SupportStateChangedEvent {
  app::SupportState state = app::SupportState::Idle;
};

struct PinAssignedEvent {
  std::string pin;
};

struct PinExpiredEvent {};

struct ErrorRaisedEvent {
  int code = 0;
  std::string message;
};

using AgentEvent = std::variant<
    WifiScanCompletedEvent,
    WifiConnectionStateChangedEvent,
    VpnStateChangedEvent,
    SupportStateChangedEvent,
    PinAssignedEvent,
    PinExpiredEvent,
    ErrorRaisedEvent>;

class AgentPort {
 public:
  virtual ~AgentPort() = default;

  [[nodiscard]] virtual app::RuntimeState get_status() = 0;
  [[nodiscard]] virtual std::vector<std::string> scan_wifi() = 0;
  virtual void connect_wifi(std::string_view ssid, std::string_view password) = 0;
  virtual void disconnect_wifi() = 0;
  virtual void start_support() = 0;
  virtual void stop_support() = 0;
  [[nodiscard]] virtual std::optional<std::string> get_pin() = 0;
  [[nodiscard]] virtual std::optional<AgentEvent> poll_event(std::chrono::milliseconds timeout) = 0;
};

}  // namespace rook::ui::ports
