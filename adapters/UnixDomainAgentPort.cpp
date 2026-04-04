#include "adapters/UnixDomainAgentPort.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <poll.h>
#include <stdexcept>
#include <string_view>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace rook::ui::adapters {

namespace {

using nlohmann::json;
using Clock = std::chrono::steady_clock;

constexpr std::string_view kPackagedAgentDefaultsFile = "/etc/default/rook-agent";
constexpr std::string_view kPackagedAgentSocket = "/run/rook-agent/agent.sock";
constexpr auto kRequestTimeout = std::chrono::seconds(2);

std::string env_or_default(const char* name, std::string_view fallback) {
  if (const char* value = std::getenv(name); value != nullptr && *value != '\0') {
    return value;
  }

  return std::string(fallback);
}

std::string trim(std::string value) {
  const auto first = value.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) {
    return {};
  }

  const auto last = value.find_last_not_of(" \t\r\n");
  value = value.substr(first, last - first + 1);
  if (value.size() >= 2 &&
      ((value.front() == '"' && value.back() == '"') || (value.front() == '\'' && value.back() == '\''))) {
    value = value.substr(1, value.size() - 2);
  }

  return value;
}

std::filesystem::path detect_user_config_root() {
  if (const char* xdg = std::getenv("XDG_CONFIG_HOME"); xdg != nullptr && *xdg != '\0') {
    return std::filesystem::path(xdg);
  }

  if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0') {
    return std::filesystem::path(home) / ".config";
  }

  return std::filesystem::current_path();
}

std::optional<std::string> read_environment_value(const std::filesystem::path& file_path, std::string_view key) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    return std::nullopt;
  }

  for (std::string line; std::getline(file, line);) {
    line = trim(std::move(line));
    if (line.empty() || line.starts_with('#')) {
      continue;
    }

    const auto delimiter = line.find('=');
    if (delimiter == std::string::npos) {
      continue;
    }

    const std::string name = trim(line.substr(0, delimiter));
    if (name != key) {
      continue;
    }

    return trim(line.substr(delimiter + 1));
  }

  return std::nullopt;
}

std::string resolve_socket_path() {
  if (const char* override_path = std::getenv("ROOK_UI_AGENT_SOCKET"); override_path != nullptr && *override_path != '\0') {
    return override_path;
  }

  if (const char* override_path = std::getenv("ROOK_UI_AGENT_REQUEST_SOCKET"); override_path != nullptr &&
      *override_path != '\0') {
    return override_path;
  }

  const std::filesystem::path defaults_file = env_or_default("ROOK_UI_AGENT_DEFAULTS_FILE", kPackagedAgentDefaultsFile);
  if (std::filesystem::exists(defaults_file)) {
    if (const auto configured = read_environment_value(defaults_file, "ROOK_AGENT_SOCKET_PATH"); configured.has_value() &&
        !configured->empty()) {
      return *configured;
    }

    return std::string(kPackagedAgentSocket);
  }

  return (detect_user_config_root() / "rook-agent" / "agent.sock").string();
}

int connect_unix_socket(const std::string& path) {
  const int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) {
    return -1;
  }

  sockaddr_un address{};
  address.sun_family = AF_UNIX;
  if (path.size() >= sizeof(address.sun_path)) {
    ::close(fd);
    errno = ENAMETOOLONG;
    return -1;
  }

  std::strncpy(address.sun_path, path.c_str(), sizeof(address.sun_path) - 1);
  if (::connect(fd, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0) {
    const int saved_errno = errno;
    ::close(fd);
    errno = saved_errno;
    return -1;
  }

  return fd;
}

void send_all(int fd, const std::string& payload) {
  std::size_t offset = 0;
  while (offset < payload.size()) {
    const ssize_t written = ::send(fd, payload.data() + offset, payload.size() - offset, 0);
    if (written < 0) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("agent request could not be written");
    }

    offset += static_cast<std::size_t>(written);
  }
}

bool is_known_event_name(std::string_view name) {
  return name == "WifiScanCompleted" || name == "WifiConnectionStateChanged" || name == "VpnStateChanged" ||
         name == "SupportStateChanged" || name == "PinAssigned" || name == "PinExpired" || name == "ErrorRaised";
}

std::string trim_line_break(std::string line) {
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  return line;
}

std::optional<json> pop_message_from_buffer(std::string& buffer) {
  while (true) {
    const auto newline = buffer.find('\n');
    if (newline == std::string::npos) {
      return std::nullopt;
    }

    std::string line = trim_line_break(buffer.substr(0, newline));
    buffer.erase(0, newline + 1);
    if (line.empty()) {
      continue;
    }

    json parsed = json::parse(line, nullptr, false);
    if (parsed.is_discarded()) {
      throw std::runtime_error("agent message could not be parsed");
    }

    return parsed;
  }
}

std::optional<json> read_message_from_stream(int fd, std::string& buffer, std::chrono::milliseconds timeout) {
  if (auto message = pop_message_from_buffer(buffer); message.has_value()) {
    return message;
  }

  const auto deadline = Clock::now() + timeout;
  std::array<char, 4096> chunk{};

  while (true) {
    const auto now = Clock::now();
    if (now >= deadline) {
      return std::nullopt;
    }

    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    pollfd descriptor{
        .fd = fd,
        .events = POLLIN,
        .revents = 0,
    };

    int poll_result = 0;
    do {
      poll_result = ::poll(&descriptor, 1, static_cast<int>(remaining.count()));
    } while (poll_result < 0 && errno == EINTR);

    if (poll_result < 0) {
      throw std::runtime_error("agent stream could not be polled");
    }
    if (poll_result == 0) {
      return std::nullopt;
    }

    const ssize_t received = ::recv(fd, chunk.data(), chunk.size(), 0);
    if (received < 0) {
      if (errno == EINTR) {
        continue;
      }
      throw std::runtime_error("agent stream could not be read");
    }
    if (received == 0) {
      throw std::runtime_error("agent connection was closed");
    }

    buffer.append(chunk.data(), static_cast<std::size_t>(received));
    if (auto message = pop_message_from_buffer(buffer); message.has_value()) {
      return message;
    }
  }
}

bool is_response_message(const json& message) {
  if (message.value("type", std::string()) == "response") {
    return true;
  }

  return message.value("direction", std::string()) == "response";
}

bool is_event_message(const json& message) {
  if (message.value("type", std::string()) == "event") {
    return true;
  }
  if (message.value("direction", std::string()) == "event") {
    return true;
  }

  return is_known_event_name(message.value("type", std::string()));
}

std::string extract_response_id(const json& response) {
  if (response.contains("id") && response["id"].is_string()) {
    return response["id"].get<std::string>();
  }

  return {};
}

std::string extract_response_action(const json& response) {
  if (response.contains("action") && response["action"].is_string()) {
    return response["action"].get<std::string>();
  }
  if (response.value("direction", std::string()) == "response") {
    return response.value("type", std::string());
  }

  return {};
}

std::string extract_event_name(const json& message) {
  if (message.value("type", std::string()) == "event") {
    return message.value("event", std::string());
  }

  return message.value("type", std::string());
}

std::string parse_error_code(const json& error) {
  if (!error.is_object()) {
    return {};
  }

  if (error.contains("code")) {
    if (error["code"].is_string()) {
      return error["code"].get<std::string>();
    }
    if (error["code"].is_number_integer()) {
      return std::to_string(error["code"].get<int>());
    }
  }

  return {};
}

std::string parse_error_message(const json& error) {
  if (!error.is_object()) {
    return {};
  }

  if (error.contains("message") && error["message"].is_string()) {
    return error["message"].get<std::string>();
  }

  return {};
}

app::ConnectionState parse_connection_state(std::string_view value) {
  return value == "connected" ? app::ConnectionState::Connected : app::ConnectionState::Disconnected;
}

app::SupportState parse_support_state(std::string_view value) {
  if (value == "online") {
    return app::SupportState::Online;
  }
  if (value == "online+vpnup") {
    return app::SupportState::OnlineVpnUp;
  }
  if (value == "servicemode") {
    return app::SupportState::ServiceMode;
  }

  return app::SupportState::Idle;
}

std::vector<std::string> parse_wifi_networks(const json& payload) {
  std::vector<std::string> result;
  if (!payload.contains("networks") || !payload["networks"].is_array()) {
    return result;
  }

  for (const auto& network : payload["networks"]) {
    if (network.contains("ssid") && network["ssid"].is_string()) {
      result.push_back(network["ssid"].get<std::string>());
    }
  }

  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

void throw_if_error_response(const json& response) {
  if (response.contains("success") && response["success"].is_boolean() && !response["success"].get<bool>()) {
    const json error = response.contains("error") ? response["error"] : json::object();
    const std::string message = parse_error_message(error);
    throw std::runtime_error(message.empty() ? "agent error" : message);
  }

  if (const auto error_iterator = response.find("error"); error_iterator != response.end() && error_iterator->is_object()) {
    const std::string message = parse_error_message(*error_iterator);
    throw std::runtime_error(message.empty() ? "agent error" : message);
  }

  const auto payload_iterator = response.find("payload");
  if (payload_iterator != response.end() && payload_iterator->is_object() && payload_iterator->contains("code") &&
      payload_iterator->contains("message")) {
    throw std::runtime_error(payload_iterator->value("message", "agent error"));
  }
}

json build_request(std::string_view id, std::string_view action, const std::optional<json>& payload = std::nullopt) {
  json request = {
      {"id", id},
      {"action", action},
  };

  if (payload.has_value()) {
    request["payload"] = *payload;
  }

  return request;
}

ports::AgentEvent parse_event(const json& message) {
  const std::string type = extract_event_name(message);
  const json payload = message.contains("payload") ? message["payload"] : json::object();

  if (type == "WifiScanCompleted") {
    return ports::WifiScanCompletedEvent{.networks = parse_wifi_networks(payload)};
  }
  if (type == "WifiConnectionStateChanged") {
    return ports::WifiConnectionStateChangedEvent{
        .state = parse_connection_state(payload.value("state", payload.value("wifiState", "disconnected"))),
    };
  }
  if (type == "VpnStateChanged") {
    return ports::VpnStateChangedEvent{
        .state = parse_connection_state(payload.value("state", payload.value("vpnState", "disconnected"))),
    };
  }
  if (type == "SupportStateChanged") {
    return ports::SupportStateChangedEvent{
        .state = parse_support_state(payload.value("state", payload.value("supportState", "idle"))),
    };
  }
  if (type == "PinAssigned") {
    return ports::PinAssignedEvent{
        .pin = payload.value(
            "pin",
            payload.contains("session") && payload["session"].is_object() ? payload["session"].value("pin", std::string()) : std::string()),
    };
  }
  if (type == "PinExpired") {
    return ports::PinExpiredEvent{};
  }

  return ports::ErrorRaisedEvent{
      .code = parse_error_code(payload),
      .message = payload.value("message", std::string("unknown agent event")),
  };
}

}  // namespace

UnixDomainAgentPort::UnixDomainAgentPort() : socket_path_(resolve_socket_path()) {}

UnixDomainAgentPort::~UnixDomainAgentPort() {
  close_connection();
}

void UnixDomainAgentPort::close_connection() {
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }

  read_buffer_.clear();
}

void UnixDomainAgentPort::ensure_connected() {
  if (fd_ >= 0) {
    return;
  }

  fd_ = connect_unix_socket(socket_path_);
  if (fd_ < 0) {
    throw std::runtime_error("agent socket could not be opened: " + std::string(std::strerror(errno)));
  }
}

std::string UnixDomainAgentPort::next_request_id() {
  return std::to_string(next_request_id_++);
}

std::optional<json> UnixDomainAgentPort::read_message(std::chrono::milliseconds timeout) {
  if (fd_ < 0) {
    return std::nullopt;
  }

  try {
    return read_message_from_stream(fd_, read_buffer_, timeout);
  } catch (...) {
    close_connection();
    throw;
  }
}

json UnixDomainAgentPort::send_request(std::string_view action, const std::optional<json>& payload) {
  std::runtime_error last_error("agent request failed");

  for (int attempt = 0; attempt < 2; ++attempt) {
    const std::string request_id = next_request_id();
    try {
      ensure_connected();
      send_all(fd_, build_request(request_id, action, payload).dump() + "\n");

      const auto deadline = Clock::now() + kRequestTimeout;
      while (true) {
        const auto now = Clock::now();
        if (now >= deadline) {
          throw std::runtime_error("agent response timed out");
        }

        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto message = read_message(remaining);
        if (!message.has_value()) {
          throw std::runtime_error("agent response was empty");
        }

        if (is_event_message(*message)) {
          pending_events_.push_back(parse_event(*message));
          continue;
        }
        if (!is_response_message(*message)) {
          continue;
        }

        if (extract_response_id(*message) != request_id) {
          throw std::runtime_error("agent response id mismatch");
        }

        const std::string response_action = extract_response_action(*message);
        if (!response_action.empty() && response_action != action) {
          throw std::runtime_error("agent response action mismatch");
        }

        throw_if_error_response(*message);
        return *message;
      }
    } catch (const std::runtime_error& error) {
      last_error = std::runtime_error(error.what());
      close_connection();
    }
  }

  throw last_error;
}

app::RuntimeState UnixDomainAgentPort::get_status() {
  const json response = send_request("GetStatus");
  const json payload = response.contains("payload") ? response["payload"] : json::object();
  app::RuntimeState state{
      .support_active = payload.value("supportActive", false),
      .support_state = parse_support_state(payload.value("supportState", "idle")),
      .wifi_state = parse_connection_state(payload.value("wifiState", "disconnected")),
      .vpn_state = parse_connection_state(payload.value("vpnState", "disconnected")),
      .any_wifi_active = payload.value("anyWifiActive", false),
      .support_wifi_active = payload.value("supportWifiActive", false),
  };

  if (payload.contains("activeWifiConnection") && payload["activeWifiConnection"].is_string()) {
    state.active_wifi_connection = payload["activeWifiConnection"].get<std::string>();
  }

  return state;
}

std::vector<std::string> UnixDomainAgentPort::scan_wifi() {
  const json response = send_request("ScanWifi");
  const json payload = response.contains("payload") ? response["payload"] : json::object();
  std::vector<std::string> networks = parse_wifi_networks(payload);
  if (!networks.empty()) {
    return networks;
  }

  const auto event = poll_event(std::chrono::milliseconds(800));
  if (event.has_value() && std::holds_alternative<ports::WifiScanCompletedEvent>(*event)) {
    return std::get<ports::WifiScanCompletedEvent>(*event).networks;
  }

  return {};
}

void UnixDomainAgentPort::connect_wifi(std::string_view ssid, std::string_view password) {
  send_request(
      "ConnectWifi",
      json{
          {"ssid", ssid},
          {"password", password},
      });
}

void UnixDomainAgentPort::disconnect_wifi() {
  send_request("DisconnectWifi");
}

void UnixDomainAgentPort::start_support() {
  send_request("StartSupport");
}

void UnixDomainAgentPort::stop_support() {
  send_request("StopSupport");
}

std::optional<std::string> UnixDomainAgentPort::get_pin() {
  const json response = send_request("GetPin");
  if (!response.contains("payload") || !response["payload"].is_object()) {
    return std::nullopt;
  }

  const json& payload = response["payload"];
  if (!payload.contains("pin") || !payload["pin"].is_string()) {
    return std::nullopt;
  }

  return payload["pin"].get<std::string>();
}

std::optional<ports::AgentEvent> UnixDomainAgentPort::poll_event(std::chrono::milliseconds timeout) {
  if (!pending_events_.empty()) {
    ports::AgentEvent event = pending_events_.front();
    pending_events_.pop_front();
    return event;
  }

  try {
    ensure_connected();
  } catch (const std::exception&) {
    return std::nullopt;
  }

  const auto deadline = Clock::now() + timeout;
  while (true) {
    const auto now = Clock::now();
    const auto remaining =
        now >= deadline ? std::chrono::milliseconds(0) : std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    const auto message = read_message(remaining);
    if (!message.has_value()) {
      return std::nullopt;
    }

    if (is_event_message(*message)) {
      return parse_event(*message);
    }

    if (Clock::now() >= deadline) {
      return std::nullopt;
    }
  }
}

}  // namespace rook::ui::adapters
