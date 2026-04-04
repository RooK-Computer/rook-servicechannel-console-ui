#pragma once

#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>

#include "ports/AgentPort.hpp"
#include "third_party/nlohmann/json.hpp"

namespace rook::ui::adapters {

class UnixDomainAgentPort final : public ports::AgentPort {
 public:
  explicit UnixDomainAgentPort(std::string socket_path = {});
  ~UnixDomainAgentPort() override;

  [[nodiscard]] app::RuntimeState get_status() override;
  [[nodiscard]] std::vector<std::string> scan_wifi() override;
  void connect_wifi(std::string_view ssid, std::string_view password) override;
  void disconnect_wifi() override;
  void start_vpn() override;
  void stop_vpn() override;
  void start_support() override;
  void stop_support() override;
  [[nodiscard]] std::optional<std::string> get_pin() override;
  [[nodiscard]] std::optional<ports::AgentEvent> poll_event(std::chrono::milliseconds timeout) override;

 private:
  void close_connection();
  void ensure_connected();
  [[nodiscard]] std::string next_request_id();
  nlohmann::json send_request(
      std::string_view action,
      const std::optional<nlohmann::json>& payload = std::nullopt,
      std::chrono::milliseconds timeout = std::chrono::seconds(2));
  [[nodiscard]] std::optional<nlohmann::json> read_message(std::chrono::milliseconds timeout);

  std::string socket_path_;
  std::string read_buffer_;
  std::deque<ports::AgentEvent> pending_events_;
  std::uint64_t next_request_id_ = 1;
  int fd_ = -1;
};

}  // namespace rook::ui::adapters
