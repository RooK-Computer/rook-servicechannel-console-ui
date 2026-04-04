#include "app/Application.hpp"

#include <iostream>

#include "app/TerminalInput.hpp"
#include "render/BackendInfo.hpp"
#include "render/RmlUiRenderer.hpp"
#include "render/TerminalRenderer.hpp"
#include "theme/Theme.hpp"

namespace rook::ui::app {

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
  if (!is_known_screen_id(resolve_start_screen())) {
    std::cerr << "Unknown screen id: " << resolve_start_screen() << '\n';
    return 2;
  }

  render::RmlUiRenderer renderer(config_.paths, config_.runtime_mode);
  if (!renderer.initialize()) {
    std::cerr << "Graphical SDL2/RmlUi host could not be started; falling back to terminal renderer.\n";
    return config_.runtime_mode == RuntimeMode::Preview ? run_preview() : run_normal();
  }

  const auto backend_info = render::detect_backend_info();
  const auto& active_theme = theme::default_theme();
  NavigationSession session(create_start_request());

  while (true) {
    const auto model = resolve_model(session.current());
    const FocusEngine focus_engine(model);
    auto focus = focus_engine.initial_state();
    renderer.render_screen(model, active_theme, backend_info, focus);

    while (true) {
      const auto command = renderer.read_command(model.show_spinner);
      if (command == InputCommand::Exit) {
        return 0;
      }

      if (command == InputCommand::None || command == InputCommand::Tick) {
        renderer.render_screen(model, active_theme, backend_info, focus);
        continue;
      }

      if (command == InputCommand::Back) {
        if (!model.allow_back) {
          renderer.render_screen(model, active_theme, backend_info, focus);
          continue;
        }
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
  const std::string& screen_id = resolve_start_screen();
  if (screen_registry_.create(screen_id) == nullptr) {
    std::cerr << "Unknown screen id: " << screen_id << '\n';
    std::cerr << "Known screen ids:";
    for (const auto& id : screen_registry_.ids()) {
      std::cerr << ' ' << id;
    }
    std::cerr << '\n';
    return 2;
  }

  const auto backend_info = render::detect_backend_info();
  const auto& active_theme = theme::default_theme();

  auto model = resolve_model(create_start_request());

  const render::TerminalRenderer renderer;
  std::cout << "config-root: " << config_.paths.config_root << '\n';
  std::cout << "settings-file: " << config_.paths.settings_file << '\n';
  std::cout << "resource-root: " << config_.paths.resource_root << '\n';
  renderer.render_screen(model, active_theme, backend_info, std::nullopt);
  return 0;
}

int Application::run() const {
  if (config_.screen_list_only) {
    return print_screen_list(std::cout);
  }

  return run_graphical();
}

}  // namespace rook::ui::app
