#include "app/Application.hpp"

#include <iostream>

#include "app/TerminalInput.hpp"
#include "render/BackendInfo.hpp"
#include "render/TerminalRenderer.hpp"
#include "theme/Theme.hpp"

namespace rook::ui::app {

Application::Application(AppConfig config)
    : config_(std::move(config)),
      preview_registry_(create_default_preview_registry(config_.paths)),
      screen_registry_(create_default_screen_registry()) {}

const std::string& Application::resolve_start_screen() const {
  if (config_.runtime_mode == RuntimeMode::Preview) {
    return config_.preview_screen_id;
  }

  return config_.initial_screen_id;
}

StartRequest Application::create_start_request() const {
  return StartRequest{
      .screen_id = resolve_start_screen(),
      .params = {},
  };
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
    const auto model = preview_registry_.model_for(session.current());
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
  if (!preview_registry_.contains(screen_id) && screen_registry_.create(screen_id) == nullptr) {
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

  render::ScreenModel model;
  if (preview_registry_.contains(screen_id)) {
    model = preview_registry_.model_for(create_start_request());
  } else {
    auto screen = screen_registry_.create(screen_id);
    model = screen->model(config_.runtime_mode);
  }
  model.footer_hint = "Normalmodus-Bootstrap. Naechste Ausbaustufe: RmlUi/SDL2-Backend.";

  const render::TerminalRenderer renderer;
  std::cout << "config-root: " << config_.paths.config_root << '\n';
  std::cout << "settings-file: " << config_.paths.settings_file << '\n';
  std::cout << "resource-root: " << config_.paths.resource_root << '\n';
  renderer.render_screen(model, active_theme, backend_info, std::nullopt);
  return 0;
}

int Application::run() const {
  if (config_.runtime_mode == RuntimeMode::Preview) {
    return run_preview();
  }

  return run_normal();
}

}  // namespace rook::ui::app
