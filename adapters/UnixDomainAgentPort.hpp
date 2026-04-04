#pragma once

#include <chrono>
#include <optional>
#include <string>

#include "ports/AgentPort.hpp"

namespace rook::ui::adapters {

class UnixDomainAgentPort final : public ports::AgentPort {
 public:
  UnixDomainAgentPort();
  ~UnixDomainAgentPort() override;

  [[nodiscard]] app::RuntimeState get_status() override;
  [[nodiscard]] std::vector<std::string> scan_wifi() override;
  void connect_wifi(std::string_view ssid, std::string_view password) override;
  void disconnect_wifi() override;
  void start_support() override;
  void stop_support() override;
  [[nodiscard]] std::optional<std::string> get_pin() override;
  [[nodiscard]] std::optional<ports::AgentEvent> poll_event(std::chrono::milliseconds timeout) override;

 private:
  std::string request_socket_path_;
  std::string event_socket_path_;
  std::string protocol_version_;
  int event_fd_ = -1;
  bool event_connect_attempted_ = false;
};

}  // namespace rook::ui::adapters
