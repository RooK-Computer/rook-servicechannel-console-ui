#include "app/Application.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <utility>

#include "adapters/UnixDomainAgentPort.hpp"
#include "app/RuntimeState.hpp"
#include "app/TerminalInput.hpp"
#include "app/UiSettings.hpp"
#include "ports/AgentPort.hpp"
#include "render/BackendInfo.hpp"
#include "render/RmlUiRenderer.hpp"
#include "render/TerminalRenderer.hpp"
#include "theme/Theme.hpp"

namespace rook::ui::app {

namespace {

using Clock = std::chrono::steady_clock;
using WifiScanFuture = std::future<std::vector<std::string>>;
using ActionFuture = std::future<void>;

std::string bool_param(bool value) {
  return value ? "true" : "false";
}

std::string join_lines(const std::vector<std::string>& lines) {
  std::string result;
  for (std::size_t index = 0; index < lines.size(); ++index) {
    if (index > 0) {
      result += '\n';
    }
    result += lines[index];
  }
  return result;
}

bool is_password_screen(std::string_view screen_id) {
  return screen_id == "keyboard" || screen_id == "password";
}

WifiScanFuture launch_wifi_scan_task(std::string socket_path) {
  return std::async(std::launch::async, [socket_path = std::move(socket_path)] {
    adapters::UnixDomainAgentPort agent(std::move(socket_path));
    return agent.scan_wifi();
  });
}

ActionFuture launch_connect_wifi_task(std::string socket_path, std::string ssid, std::string password) {
  return std::async(std::launch::async, [socket_path = std::move(socket_path), ssid = std::move(ssid), password = std::move(password)] {
    adapters::UnixDomainAgentPort agent(std::move(socket_path));
    agent.connect_wifi(ssid, password);
  });
}

ActionFuture launch_start_support_task(std::string socket_path) {
  return std::async(std::launch::async, [socket_path = std::move(socket_path)] {
    adapters::UnixDomainAgentPort agent(std::move(socket_path));
    agent.start_support();
  });
}

std::string utf8_pop_back(std::string value) {
  while (!value.empty()) {
    const unsigned char byte = static_cast<unsigned char>(value.back());
    value.pop_back();
    if ((byte & 0xC0) != 0x80) {
      break;
    }
  }
  return value;
}

void apply_event(RuntimeState& runtime, const ports::AgentEvent& event) {
  if (const auto* wifi_scan = std::get_if<ports::WifiScanCompletedEvent>(&event); wifi_scan != nullptr) {
    runtime.wifi_networks = wifi_scan->networks;
    return;
  }

  if (const auto* wifi_state = std::get_if<ports::WifiConnectionStateChangedEvent>(&event); wifi_state != nullptr) {
    runtime.wifi_state = wifi_state->state;
    runtime.any_wifi_active = wifi_state->state == ConnectionState::Connected;
    runtime.support_wifi_active = wifi_state->state == ConnectionState::Connected;
    return;
  }

  if (const auto* vpn_state = std::get_if<ports::VpnStateChangedEvent>(&event); vpn_state != nullptr) {
    runtime.vpn_state = vpn_state->state;
    return;
  }

  if (const auto* support_state = std::get_if<ports::SupportStateChangedEvent>(&event); support_state != nullptr) {
    runtime.support_active = support_state->runtime.support_active;
    runtime.support_state = support_state->runtime.support_state;
    runtime.wifi_state = support_state->runtime.wifi_state;
    runtime.vpn_state = support_state->runtime.vpn_state;
    runtime.any_wifi_active = support_state->runtime.any_wifi_active;
    runtime.support_wifi_active = support_state->runtime.support_wifi_active;
    runtime.active_wifi_connection = support_state->runtime.active_wifi_connection;
    if (!support_state->runtime.wifi_networks.empty()) {
      runtime.wifi_networks = support_state->runtime.wifi_networks;
    }
    if (support_state->runtime.pin.has_value()) {
      runtime.pin = support_state->runtime.pin;
    }
    return;
  }

  if (const auto* pin_assigned = std::get_if<ports::PinAssignedEvent>(&event); pin_assigned != nullptr) {
    runtime.pin = pin_assigned->pin;
    return;
  }

  if (std::holds_alternative<ports::PinExpiredEvent>(event)) {
    runtime.pin.reset();
    return;
  }

  if (const auto* error = std::get_if<ports::ErrorRaisedEvent>(&event); error != nullptr) {
    runtime.last_error = RuntimeError{
        .code = error->code,
        .message = error->message,
    };
  }
}

void refresh_runtime_status(ports::AgentPort& agent, RuntimeState& runtime) {
  RuntimeState latest = agent.get_status();
  if (latest.wifi_networks.empty()) {
    latest.wifi_networks = runtime.wifi_networks;
  }
  latest.last_error = runtime.last_error;

  if (has_active_support_session(latest)) {
    if (!latest.pin.has_value()) {
      latest.pin = runtime.pin;
    }
    if (!latest.pin.has_value()) {
      const auto pin = agent.get_pin();
      if (pin.has_value()) {
        latest.pin = *pin;
      }
    }
  } else {
    latest.pin.reset();
  }

  runtime = std::move(latest);
}

StartRequest determine_normal_start_request(const UiSettings& settings, const RuntimeState& runtime) {
  if (!settings.hide_welcome) {
    return StartRequest{
        .screen_id = "welcome",
        .params = {},
    };
  }

  if (has_active_support_session(runtime)) {
    return StartRequest{
        .screen_id = "status",
        .params = {},
    };
  }

  if (has_connected_wifi(runtime)) {
    return StartRequest{
        .screen_id = "vpn-wait",
        .params = {},
    };
  }

  return StartRequest{
      .screen_id = "wifi-scan",
      .params = {},
  };
}

IntentParams runtime_params_for_request(const StartRequest& request, const UiSettings& settings, const RuntimeState& runtime) {
  IntentParams params = request.params;

  if (request.screen_id == "welcome") {
    params["hide-welcome"] = bool_param(settings.hide_welcome);
    params["continue-target"] = has_active_support_session(runtime) ? "status" : (has_connected_wifi(runtime) ? "vpn-wait" : "wifi-scan");
  } else if (request.screen_id == "status") {
    if (runtime.pin.has_value()) {
      params["pin"] = *runtime.pin;
    }
  } else if (request.screen_id == "wifi-list") {
    if (runtime.wifi_networks.empty()) {
      params["empty"] = "true";
    } else {
      params.erase("empty");
      params["networks"] = join_lines(runtime.wifi_networks);
    }
  }

  if ((request.screen_id == "wifi-wait" || is_password_screen(request.screen_id)) && !params.contains("ssid") &&
      runtime.active_wifi_connection.has_value()) {
    params["ssid"] = *runtime.active_wifi_connection;
  }

  return params;
}

bool wants_periodic_updates(const StartRequest& request, const render::ScreenModel& model, RuntimeMode runtime_mode) {
  if (model.show_spinner) {
    return true;
  }

  if (runtime_mode != RuntimeMode::Normal) {
    return false;
  }

  return request.screen_id == "wifi-list" || request.screen_id == "status";
}

}  // namespace

Application::Application(AppConfig config)
    : config_(std::move(config)),
      preview_registry_(create_default_preview_registry(config_.paths)),
      screen_registry_(create_default_screen_registry(config_.paths)) {}

const std::string& Application::resolve_start_screen() const {
  if (config_.runtime_mode == RuntimeMode::Preview) {
    return config_.preview_screen_id;
  }

  return config_.initial_screen_id;
}

StartRequest Application::create_start_request() const {
  if (config_.runtime_mode == RuntimeMode::Preview) {
    return preview_registry_.start_request_for(resolve_start_screen());
  }

  return StartRequest{
      .screen_id = resolve_start_screen(),
      .params = {},
  };
}

bool Application::is_known_screen_id(std::string_view screen_id) const {
  if (config_.runtime_mode == RuntimeMode::Preview && preview_registry_.contains(screen_id)) {
    return true;
  }

  return screen_registry_.contains(screen_id);
}

render::ScreenModel Application::resolve_model(const StartRequest& request) const {
  auto screen = screen_registry_.create(request.screen_id);
  if (screen == nullptr) {
    throw std::runtime_error("unknown screen id");
  }

  return screen->model(screens::ScreenContext{
      .runtime_mode = config_.runtime_mode,
      .params = request.params,
  });
}

int Application::print_screen_list(std::ostream& out) const {
  for (const auto& id : screen_registry_.ids()) {
    out << id << '\n';
  }

  return 0;
}

int Application::run_graphical() const {
  if (config_.runtime_mode == RuntimeMode::Preview && !is_known_screen_id(resolve_start_screen())) {
    std::cerr << "Unknown screen id: " << resolve_start_screen() << '\n';
    return 2;
  }

  render::RmlUiRenderer renderer(config_.paths, config_.runtime_mode);
  if (!renderer.initialize()) {
    if (config_.runtime_mode == RuntimeMode::Preview && !config_.screenshot_path.empty()) {
      std::cerr << "Preview screenshot requires the graphical SDL2/RmlUi host.\n";
      return 2;
    }
    std::cerr << "Graphical SDL2/RmlUi host could not be started; falling back to terminal renderer.\n";
    return config_.runtime_mode == RuntimeMode::Preview ? run_preview() : run_normal();
  }

  const auto backend_info = render::detect_backend_info();
  const auto& active_theme = theme::default_theme();

  UiSettings settings;
  std::unique_ptr<ports::AgentPort> agent;
  RuntimeState runtime;
  std::optional<WifiScanFuture> wifi_scan_future;
  std::optional<ActionFuture> wifi_connect_future;
  std::optional<ActionFuture> start_support_future;
  StartRequest start_request = create_start_request();

  if (config_.runtime_mode == RuntimeMode::Normal) {
    UiSettingsStore settings_store(config_.paths.settings_file);
    settings = settings_store.load();
    agent = std::make_unique<adapters::UnixDomainAgentPort>(config_.agent_socket_path);
    refresh_runtime_status(*agent, runtime);
    start_request = determine_normal_start_request(settings, runtime);

  }

  NavigationSession session(start_request);
  auto screen_entered_at = Clock::now();
  auto last_status_refresh = Clock::now();

  while (true) {
    StartRequest active_request = session.current();
    if (config_.runtime_mode == RuntimeMode::Normal && agent != nullptr) {
      if (active_request.screen_id == "wifi-scan" && !wifi_scan_future.has_value()) {
        runtime.last_error.reset();
        runtime.wifi_networks.clear();
        wifi_scan_future.emplace(launch_wifi_scan_task(config_.agent_socket_path));
      }
      if (active_request.screen_id == "wifi-wait" && !wifi_connect_future.has_value()) {
        runtime.last_error.reset();
        runtime.active_wifi_connection = active_request.params.at("ssid");
        wifi_connect_future.emplace(
            launch_connect_wifi_task(config_.agent_socket_path, active_request.params.at("ssid"), active_request.params.at("password")));
      }
      if (active_request.screen_id == "vpn-wait" && !start_support_future.has_value()) {
        runtime.last_error.reset();
        start_support_future.emplace(launch_start_support_task(config_.agent_socket_path));
      }
      active_request.params = runtime_params_for_request(active_request, settings, runtime);
    }

    const auto model = resolve_model(active_request);
    const FocusEngine focus_engine(model);
    auto focus = focus_engine.initial_state();
    renderer.render_screen(model, active_theme, backend_info, focus);
    if (!config_.screenshot_path.empty()) {
      return renderer.capture_screenshot(config_.screenshot_path) ? 0 : 2;
    }

    while (true) {
      const auto command = renderer.read_command(wants_periodic_updates(active_request, model, config_.runtime_mode));
      if (command == InputCommand::Exit) {
        return 0;
      }

      if (config_.runtime_mode == RuntimeMode::Normal && agent != nullptr &&
          (command == InputCommand::Tick || active_request.screen_id == "wifi-list" || active_request.screen_id == "status" ||
           active_request.screen_id == "wifi-scan")) {
        for (auto event = agent->poll_event(std::chrono::milliseconds(0)); event.has_value();
             event = agent->poll_event(std::chrono::milliseconds(0))) {
          apply_event(runtime, *event);
        }

        const auto now = Clock::now();
        if (now - last_status_refresh >= std::chrono::seconds(1) ||
            active_request.screen_id == "wifi-wait" ||
            active_request.screen_id == "vpn-wait") {
          refresh_runtime_status(*agent, runtime);
          last_status_refresh = now;
        }

        if (active_request.screen_id == "wifi-wait") {
          if (wifi_connect_future.has_value() &&
              wifi_connect_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            try {
              wifi_connect_future->get();
              wifi_connect_future.reset();
              runtime.last_error.reset();
            } catch (const std::exception& error) {
              wifi_connect_future.reset();
              session.apply(navigate_to("wifi-error", {{"message", error.what()}}));
              screen_entered_at = now;
              break;
            }
          }

          if (has_connected_wifi(runtime)) {
            start_support_future.reset();
            session.apply(navigate_to("vpn-wait"));
            screen_entered_at = now;
            break;
          }

          if (runtime.last_error.has_value() || now - screen_entered_at >= std::chrono::seconds(30)) {
            IntentParams params;
            if (runtime.last_error.has_value()) {
              params["message"] = runtime.last_error->message;
            }
            session.apply(navigate_to("wifi-error", params));
            screen_entered_at = now;
            break;
          }
        }

        if (active_request.screen_id == "vpn-wait") {
          if (start_support_future.has_value() &&
              start_support_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            try {
              start_support_future->get();
              start_support_future.reset();
              runtime.last_error.reset();
            } catch (const std::exception& error) {
              start_support_future.reset();
              session.apply(navigate_to("vpn-error", {{"message", error.what()}}));
              screen_entered_at = now;
              break;
            }
          }

          if (has_active_support_session(runtime) && runtime.pin.has_value()) {
            session.apply(navigate_to("status", {{"pin", *runtime.pin}}));
            screen_entered_at = now;
            break;
          }

          if (runtime.last_error.has_value() || now - screen_entered_at >= std::chrono::seconds(60)) {
            IntentParams params;
            if (runtime.last_error.has_value()) {
              params["message"] = runtime.last_error->message;
            }
            session.apply(navigate_to("vpn-error", params));
            screen_entered_at = now;
            break;
          }
        }

        if (active_request.screen_id == "wifi-scan" && wifi_scan_future.has_value() &&
            wifi_scan_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
          try {
            runtime.wifi_networks = wifi_scan_future->get();
            runtime.last_error.reset();
            wifi_scan_future.reset();
            session.apply(navigate_to("wifi-list"));
          } catch (const std::exception& error) {
            wifi_scan_future.reset();
            session.apply(navigate_to("wifi-error", {{"message", error.what()}}));
          }
          screen_entered_at = now;
          break;
        }

        if (command == InputCommand::Tick) {
          StartRequest refreshed_request = session.current();
          refreshed_request.params = runtime_params_for_request(refreshed_request, settings, runtime);
          renderer.render_screen(resolve_model(refreshed_request), active_theme, backend_info, focus);
          continue;
        }
      }

      if (command == InputCommand::None) {
        renderer.render_screen(model, active_theme, backend_info, focus);
        continue;
      }

      if (command == InputCommand::Back) {
        if (is_password_screen(active_request.screen_id)) {
          const auto password_iterator = active_request.params.find("password");
          if (password_iterator != active_request.params.end() && !password_iterator->second.empty()) {
            StartRequest updated_request = active_request;
            updated_request.screen_id = "keyboard";
            updated_request.params["password"] = utf8_pop_back(password_iterator->second);
            updated_request.params["focus-key"] = "backspace";
            session.replace_current(std::move(updated_request));
            screen_entered_at = Clock::now();
            break;
          }
        }

        if (!model.allow_back) {
          renderer.render_screen(model, active_theme, backend_info, focus);
          continue;
        }
        if (!session.go_back()) {
          return 0;
        }
        screen_entered_at = Clock::now();
        break;
      }

      if (command == InputCommand::Confirm) {
        const auto intent = focus_engine.intent_for(focus);
        if (!intent.has_value()) {
          continue;
        }

        if (intent->kind == IntentKind::GoBack) {
          if (!session.go_back()) {
            return 0;
          }
          screen_entered_at = Clock::now();
          break;
        }

        if (is_password_screen(active_request.screen_id) &&
            intent->kind == IntentKind::NavigateTo &&
            is_password_screen(intent->target_screen_id)) {
          session.replace_current(StartRequest{
              .screen_id = "keyboard",
              .params = intent->params,
          });
          screen_entered_at = Clock::now();
          break;
        }

        if (config_.runtime_mode == RuntimeMode::Normal && agent != nullptr) {
          if (active_request.screen_id == "welcome" && intent->kind == IntentKind::NavigateTo && intent->target_screen_id == "welcome") {
            settings.hide_welcome = intent->params.contains("hide-welcome") &&
                                    (intent->params.at("hide-welcome") == "true" || intent->params.at("hide-welcome") == "1");
            UiSettingsStore(config_.paths.settings_file).save(settings);
          }

          if (is_password_screen(active_request.screen_id) && intent->kind == IntentKind::NavigateTo && intent->target_screen_id == "wifi-wait") {
            const std::string ssid = intent->params.at("ssid");
            runtime.active_wifi_connection = ssid;
            runtime.last_error.reset();
          }

          if (active_request.screen_id == "status" &&
              active_request.params.contains("dialog") &&
              active_request.params.at("dialog") == "disconnect" &&
              intent->kind == IntentKind::NavigateTo &&
              intent->target_screen_id == "welcome") {
            agent->stop_support();
            agent->disconnect_wifi();
            runtime = RuntimeState{};
          }

          if ((active_request.screen_id == "wifi-error" || active_request.screen_id == "vpn-error") &&
              intent->kind == IntentKind::NavigateTo &&
              intent->target_screen_id == "wifi-scan") {
            runtime.last_error.reset();
            runtime.wifi_networks.clear();
          }
        }

        if (!session.apply(*intent)) {
          return 0;
        }

        screen_entered_at = Clock::now();
        break;
      }

      focus = focus_engine.move(focus, command);
      renderer.render_screen(model, active_theme, backend_info, focus);
    }
  }
}

int Application::run_preview() const {
  NavigationSession session(create_start_request());
  if (!preview_registry_.contains(session.current().screen_id)) {
    std::cerr << "Unknown preview screen id: " << session.current().screen_id << '\n';
    std::cerr << "Known preview screen ids:";
    for (const auto& id : preview_registry_.ids()) {
      std::cerr << ' ' << id;
    }
    std::cerr << '\n';
    return 2;
  }

  const auto backend_info = render::detect_backend_info();
  const auto& active_theme = theme::default_theme();
  const render::TerminalRenderer renderer;
  const TerminalInput input;

  while (true) {
    const auto model = resolve_model(session.current());
    const FocusEngine focus_engine(model);
    auto focus = focus_engine.initial_state();
    renderer.render_screen(model, active_theme, backend_info, focus);

    while (true) {
      const auto command = input.read_command();
      if (command == InputCommand::Exit) {
        return 0;
      }

      if (command == InputCommand::Back) {
        if (!session.go_back()) {
          return 0;
        }
        break;
      }

      if (command == InputCommand::Confirm) {
        const auto intent = focus_engine.intent_for(focus);
        if (!intent.has_value()) {
          continue;
        }

        if (intent->kind == IntentKind::GoBack) {
          if (!session.go_back()) {
            return 0;
          }
          break;
        }

        if (!session.apply(*intent)) {
          return 0;
        }
        break;
      }

      focus = focus_engine.move(focus, command);
      renderer.render_screen(model, active_theme, backend_info, focus);
    }
  }
}

int Application::run_normal() const {
  const auto backend_info = render::detect_backend_info();
  const auto& active_theme = theme::default_theme();
  const render::TerminalRenderer renderer;
  const TerminalInput input;

  UiSettingsStore settings_store(config_.paths.settings_file);
  UiSettings settings = settings_store.load();
  auto agent = std::make_unique<adapters::UnixDomainAgentPort>(config_.agent_socket_path);
  RuntimeState runtime;
  std::optional<WifiScanFuture> wifi_scan_future;
  std::optional<ActionFuture> wifi_connect_future;
  std::optional<ActionFuture> start_support_future;
  refresh_runtime_status(*agent, runtime);
  StartRequest start_request = determine_normal_start_request(settings, runtime);

  NavigationSession session(start_request);
  auto screen_entered_at = Clock::now();
  auto last_status_refresh = Clock::now();

  std::cout << "config-root: " << config_.paths.config_root << '\n';
  std::cout << "settings-file: " << config_.paths.settings_file << '\n';
  std::cout << "resource-root: " << config_.paths.resource_root << '\n';

  while (true) {
    StartRequest active_request = session.current();
    if (active_request.screen_id == "wifi-scan" && !wifi_scan_future.has_value()) {
      runtime.last_error.reset();
      runtime.wifi_networks.clear();
      wifi_scan_future.emplace(launch_wifi_scan_task(config_.agent_socket_path));
    }
    if (active_request.screen_id == "wifi-wait" && !wifi_connect_future.has_value()) {
      runtime.last_error.reset();
      runtime.active_wifi_connection = active_request.params.at("ssid");
      wifi_connect_future.emplace(
          launch_connect_wifi_task(config_.agent_socket_path, active_request.params.at("ssid"), active_request.params.at("password")));
    }
    if (active_request.screen_id == "vpn-wait" && !start_support_future.has_value()) {
      runtime.last_error.reset();
      start_support_future.emplace(launch_start_support_task(config_.agent_socket_path));
    }
    active_request.params = runtime_params_for_request(active_request, settings, runtime);

    const auto model = resolve_model(active_request);
    const FocusEngine focus_engine(model);
    auto focus = focus_engine.initial_state();
    renderer.render_screen(model, active_theme, backend_info, focus);

    while (true) {
      const auto command =
          input.read_command(wants_periodic_updates(active_request, model, config_.runtime_mode) ? std::optional{std::chrono::milliseconds(100)}
                                                                                                  : std::nullopt);
      if (command == InputCommand::Exit) {
        return 0;
      }

      for (auto event = agent->poll_event(std::chrono::milliseconds(0)); event.has_value();
           event = agent->poll_event(std::chrono::milliseconds(0))) {
        apply_event(runtime, *event);
      }

      const auto now = Clock::now();
      if (now - last_status_refresh >= std::chrono::seconds(1) ||
          active_request.screen_id == "wifi-list" ||
          active_request.screen_id == "wifi-wait" ||
          active_request.screen_id == "status") {
        refresh_runtime_status(*agent, runtime);
        last_status_refresh = now;
      }

      if (active_request.screen_id == "wifi-wait") {
        if (wifi_connect_future.has_value() &&
            wifi_connect_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
          try {
            wifi_connect_future->get();
            wifi_connect_future.reset();
            runtime.last_error.reset();
          } catch (const std::exception& error) {
            wifi_connect_future.reset();
            session.apply(navigate_to("wifi-error", {{"message", error.what()}}));
            screen_entered_at = now;
            break;
          }
        }

        if (has_connected_wifi(runtime)) {
          start_support_future.reset();
          session.apply(navigate_to("vpn-wait"));
          screen_entered_at = now;
          break;
        }

        if (runtime.last_error.has_value() || now - screen_entered_at >= std::chrono::seconds(30)) {
          IntentParams params;
          if (runtime.last_error.has_value()) {
            params["message"] = runtime.last_error->message;
          }
          session.apply(navigate_to("wifi-error", params));
          screen_entered_at = now;
          break;
        }
      }

      if (active_request.screen_id == "vpn-wait") {
        if (start_support_future.has_value() &&
            start_support_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
          try {
            start_support_future->get();
            start_support_future.reset();
            runtime.last_error.reset();
          } catch (const std::exception& error) {
            start_support_future.reset();
            session.apply(navigate_to("vpn-error", {{"message", error.what()}}));
            screen_entered_at = now;
            break;
          }
        }

        if (has_active_support_session(runtime) && runtime.pin.has_value()) {
          session.apply(navigate_to("status", {{"pin", *runtime.pin}}));
          screen_entered_at = now;
          break;
        }

        if (runtime.last_error.has_value() || now - screen_entered_at >= std::chrono::seconds(60)) {
          IntentParams params;
          if (runtime.last_error.has_value()) {
            params["message"] = runtime.last_error->message;
          }
          session.apply(navigate_to("vpn-error", params));
          screen_entered_at = now;
          break;
        }
      }

      if (active_request.screen_id == "wifi-scan" && wifi_scan_future.has_value() &&
          wifi_scan_future->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
        try {
          runtime.wifi_networks = wifi_scan_future->get();
          runtime.last_error.reset();
          wifi_scan_future.reset();
          session.apply(navigate_to("wifi-list"));
        } catch (const std::exception& error) {
          wifi_scan_future.reset();
          session.apply(navigate_to("wifi-error", {{"message", error.what()}}));
        }
        screen_entered_at = now;
        break;
      }

      if (command == InputCommand::Tick) {
        StartRequest refreshed_request = session.current();
        refreshed_request.params = runtime_params_for_request(refreshed_request, settings, runtime);
        renderer.render_screen(resolve_model(refreshed_request), active_theme, backend_info, focus);
        continue;
      }

      if (command == InputCommand::None) {
        renderer.render_screen(model, active_theme, backend_info, focus);
        continue;
      }

      if (command == InputCommand::Back) {
        if (is_password_screen(active_request.screen_id)) {
          const auto password_iterator = active_request.params.find("password");
          if (password_iterator != active_request.params.end() && !password_iterator->second.empty()) {
            StartRequest updated_request = active_request;
            updated_request.screen_id = "keyboard";
            updated_request.params["password"] = utf8_pop_back(password_iterator->second);
            updated_request.params["focus-key"] = "backspace";
            session.replace_current(std::move(updated_request));
            screen_entered_at = Clock::now();
            break;
          }
        }

        if (!model.allow_back) {
          renderer.render_screen(model, active_theme, backend_info, focus);
          continue;
        }
        if (!session.go_back()) {
          return 0;
        }
        screen_entered_at = Clock::now();
        break;
      }

      if (command == InputCommand::Confirm) {
        const auto intent = focus_engine.intent_for(focus);
        if (!intent.has_value()) {
          continue;
        }

        if (intent->kind == IntentKind::GoBack) {
          if (!session.go_back()) {
            return 0;
          }
          screen_entered_at = Clock::now();
          break;
        }

        if (is_password_screen(active_request.screen_id) &&
            intent->kind == IntentKind::NavigateTo &&
            is_password_screen(intent->target_screen_id)) {
          session.replace_current(StartRequest{
              .screen_id = "keyboard",
              .params = intent->params,
          });
          screen_entered_at = Clock::now();
          break;
        }

        if (active_request.screen_id == "welcome" && intent->kind == IntentKind::NavigateTo && intent->target_screen_id == "welcome") {
          settings.hide_welcome = intent->params.contains("hide-welcome") &&
                                  (intent->params.at("hide-welcome") == "true" || intent->params.at("hide-welcome") == "1");
          settings_store.save(settings);
        }

        if (is_password_screen(active_request.screen_id) && intent->kind == IntentKind::NavigateTo && intent->target_screen_id == "wifi-wait") {
          const std::string ssid = intent->params.at("ssid");
          runtime.active_wifi_connection = ssid;
          runtime.last_error.reset();
        }

        if (active_request.screen_id == "status" &&
            active_request.params.contains("dialog") &&
            active_request.params.at("dialog") == "disconnect" &&
            intent->kind == IntentKind::NavigateTo &&
            intent->target_screen_id == "welcome") {
          agent->stop_support();
          agent->disconnect_wifi();
          runtime = RuntimeState{};
        }

        if ((active_request.screen_id == "wifi-error" || active_request.screen_id == "vpn-error") &&
            intent->kind == IntentKind::NavigateTo &&
            intent->target_screen_id == "wifi-scan") {
          runtime.last_error.reset();
          runtime.wifi_networks.clear();
        }

        if (!session.apply(*intent)) {
          return 0;
        }

        screen_entered_at = Clock::now();
        break;
      }

      focus = focus_engine.move(focus, command);
      renderer.render_screen(model, active_theme, backend_info, focus);
    }
  }
}

int Application::run() const {
  if (config_.screen_list_only) {
    return print_screen_list(std::cout);
  }

  try {
    return run_graphical();
  } catch (const std::exception& error) {
    std::cerr << "UI runtime failed: " << error.what() << '\n';
    return 2;
  }
}

}  // namespace rook::ui::app
