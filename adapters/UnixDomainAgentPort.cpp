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

#include "third_party/nlohmann/json.hpp"

namespace rook::ui::adapters {

namespace {

using nlohmann::json;

constexpr std::string_view kDefaultVersion = "0.2.0-draft";
constexpr std::string_view kEventSocket = "/run/rook-servicechannel/rook-service-channel-events";
constexpr std::string_view kPackagedAgentDefaultsFile = "/etc/default/rook-agent";
constexpr std::string_view kPackagedAgentSocket = "/run/rook-agent/agent.sock";

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

std::string resolve_request_socket_path() {
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

int connect_unix_socket(const std::string& path, int socket_type) {
  const int fd = ::socket(AF_UNIX, socket_type, 0);
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
      throw std::runtime_error("agent request could not be written");
    }

    offset += static_cast<std::size_t>(written);
  }
}

json receive_json_from_seqpacket(int fd) {
  std::array<char, 65536> buffer{};
  const ssize_t received = ::recv(fd, buffer.data(), buffer.size(), 0);
  if (received <= 0) {
    throw std::runtime_error("agent response was empty");
  }

  return json::parse(std::string_view(buffer.data(), static_cast<std::size_t>(received)));
}

json receive_json_from_stream(int fd) {
  std::string buffer;
  std::array<char, 4096> chunk{};

  while (true) {
    const ssize_t received = ::recv(fd, chunk.data(), chunk.size(), 0);
    if (received < 0) {
      throw std::runtime_error("agent response could not be read");
    }

    if (received == 0) {
      break;
    }

    buffer.append(chunk.data(), static_cast<std::size_t>(received));
    json parsed = json::parse(buffer, nullptr, false);
    if (!parsed.is_discarded()) {
      return parsed;
    }
  }

  if (buffer.empty()) {
    throw std::runtime_error("agent response was empty");
  }

  return json::parse(buffer);
}

json exchange_request(const std::string& socket_path, const json& request) {
  const std::string payload = request.dump();
  std::string last_error;

  for (const int socket_type : {SOCK_SEQPACKET, SOCK_STREAM}) {
    const int fd = connect_unix_socket(socket_path, socket_type);
    if (fd < 0) {
      last_error = std::strerror(errno);
      continue;
    }

    try {
      timeval timeout{
          .tv_sec = 2,
          .tv_usec = 0,
      };
      ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
      ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

      send_all(fd, payload);
      if (socket_type == SOCK_STREAM) {
        ::shutdown(fd, SHUT_WR);
      }

      json response = socket_type == SOCK_SEQPACKET ? receive_json_from_seqpacket(fd) : receive_json_from_stream(fd);
      ::close(fd);
      return response;
    } catch (...) {
      ::close(fd);
      throw;
    }
  }

  throw std::runtime_error("agent socket could not be opened: " + last_error);
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
  const auto payload_iterator = response.find("payload");
  if (payload_iterator == response.end() || !payload_iterator->is_object()) {
    return;
  }

  if (payload_iterator->contains("code") && payload_iterator->contains("message")) {
    throw std::runtime_error(payload_iterator->value("message", "agent error"));
  }
}

json build_request(std::string_view version, std::string_view type, const std::optional<json>& payload = std::nullopt) {
  json request = {
      {"version", version},
      {"type", type},
      {"direction", "request"},
  };

  if (payload.has_value()) {
    request["payload"] = *payload;
  }

  return request;
}

ports::AgentEvent parse_event(const json& message) {
  const std::string type = message.value("type", "");
  const json payload = message.contains("payload") ? message["payload"] : json::object();

  if (type == "WifiScanCompleted") {
    return ports::WifiScanCompletedEvent{.networks = parse_wifi_networks(payload)};
  }
  if (type == "WifiConnectionStateChanged") {
    return ports::WifiConnectionStateChangedEvent{
        .state = parse_connection_state(payload.value("state", "disconnected")),
    };
  }
  if (type == "VpnStateChanged") {
    return ports::VpnStateChangedEvent{
        .state = parse_connection_state(payload.value("state", "disconnected")),
    };
  }
  if (type == "SupportStateChanged") {
    return ports::SupportStateChangedEvent{
        .state = parse_support_state(payload.value("state", "idle")),
    };
  }
  if (type == "PinAssigned") {
    return ports::PinAssignedEvent{
        .pin = payload.value("pin", ""),
    };
  }
  if (type == "PinExpired") {
    return ports::PinExpiredEvent{};
  }

  return ports::ErrorRaisedEvent{
      .code = payload.value("code", 0),
      .message = payload.value("message", std::string("unknown agent event")),
  };
}

}  // namespace

UnixDomainAgentPort::UnixDomainAgentPort()
    : request_socket_path_(resolve_request_socket_path()),
      event_socket_path_(env_or_default("ROOK_UI_AGENT_EVENT_SOCKET", kEventSocket)),
      protocol_version_(kDefaultVersion) {}

UnixDomainAgentPort::~UnixDomainAgentPort() {
  if (event_fd_ >= 0) {
    ::close(event_fd_);
  }
}

app::RuntimeState UnixDomainAgentPort::get_status() {
  const json response = exchange_request(request_socket_path_, build_request(protocol_version_, "GetStatus"));
  throw_if_error_response(response);

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
  const json response = exchange_request(request_socket_path_, build_request(protocol_version_, "ScanWifi"));
  throw_if_error_response(response);

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
  exchange_request(
      request_socket_path_,
      build_request(
          protocol_version_,
          "ConnectWifi",
          json{
              {"ssid", ssid},
              {"password", password},
          }));
}

void UnixDomainAgentPort::disconnect_wifi() {
  exchange_request(request_socket_path_, build_request(protocol_version_, "DisconnectWifi"));
}

void UnixDomainAgentPort::start_support() {
  exchange_request(request_socket_path_, build_request(protocol_version_, "StartSupport"));
}

void UnixDomainAgentPort::stop_support() {
  exchange_request(request_socket_path_, build_request(protocol_version_, "StopSupport"));
}

std::optional<std::string> UnixDomainAgentPort::get_pin() {
  const json response = exchange_request(request_socket_path_, build_request(protocol_version_, "GetPin"));
  throw_if_error_response(response);

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
  if (!event_connect_attempted_) {
    event_connect_attempted_ = true;
    event_fd_ = connect_unix_socket(event_socket_path_, SOCK_SEQPACKET);
  }

  if (event_fd_ < 0) {
    return std::nullopt;
  }

  pollfd descriptor{
      .fd = event_fd_,
      .events = POLLIN,
      .revents = 0,
  };

  const int result = ::poll(&descriptor, 1, static_cast<int>(timeout.count()));
  if (result <= 0 || (descriptor.revents & POLLIN) == 0) {
    return std::nullopt;
  }

  std::array<char, 65536> buffer{};
  const ssize_t received = ::recv(event_fd_, buffer.data(), buffer.size(), 0);
  if (received <= 0) {
    return std::nullopt;
  }

  return parse_event(json::parse(std::string_view(buffer.data(), static_cast<std::size_t>(received))));
}

}  // namespace rook::ui::adapters
