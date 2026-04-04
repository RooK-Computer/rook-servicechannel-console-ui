#include "render/RmlUiRenderer.hpp"

#if defined(ROOK_UI_HAS_SDL2) && defined(ROOK_UI_HAS_RMLUI)

#include <SDL.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/StringUtilities.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rook::ui::render {

namespace {

constexpr int kPreviewWindowWidth = 1280;
constexpr int kPreviewWindowHeight = 720;
constexpr int kNormalWindowWidth = 1280;
constexpr int kNormalWindowHeight = 720;
constexpr Sint16 kAxisPressThreshold = 16000;
constexpr Sint16 kAxisReleaseThreshold = 8000;

std::vector<std::string> available_video_drivers() {
  std::vector<std::string> drivers;
  const int count = SDL_GetNumVideoDrivers();
  drivers.reserve(static_cast<std::size_t>(std::max(count, 0)));

  for (int index = 0; index < count; ++index) {
    if (const char* driver = SDL_GetVideoDriver(index); driver != nullptr) {
      drivers.emplace_back(driver);
    }
  }

  return drivers;
}

bool contains_driver(const std::vector<std::string>& drivers, std::string_view driver) {
  return std::find(drivers.begin(), drivers.end(), driver) != drivers.end();
}

std::vector<std::string> preferred_video_drivers(app::RuntimeMode runtime_mode, const std::vector<std::string>& available_drivers) {
  std::vector<std::string> preferred;

  const auto append_if_available = [&](std::string_view driver) {
    if (contains_driver(available_drivers, driver) &&
        std::find(preferred.begin(), preferred.end(), driver) == preferred.end()) {
      preferred.emplace_back(driver);
    }
  };

  const char* session_type = std::getenv("XDG_SESSION_TYPE");
  const bool prefer_wayland = session_type != nullptr && std::string_view(session_type) == "wayland";
  const bool prefer_x11 = session_type != nullptr && std::string_view(session_type) == "x11";

  if (runtime_mode == app::RuntimeMode::Preview) {
    if (prefer_wayland) {
      append_if_available("wayland");
      append_if_available("x11");
    } else if (prefer_x11) {
      append_if_available("x11");
      append_if_available("wayland");
    } else {
      append_if_available("wayland");
      append_if_available("x11");
    }
    append_if_available("kmsdrm");
  } else {
    append_if_available("kmsdrm");
    if (prefer_wayland) {
      append_if_available("wayland");
      append_if_available("x11");
    } else if (prefer_x11) {
      append_if_available("x11");
      append_if_available("wayland");
    } else {
      append_if_available("wayland");
      append_if_available("x11");
    }
  }

  for (const auto& driver : available_drivers) {
    if (std::find(preferred.begin(), preferred.end(), driver) == preferred.end()) {
      preferred.push_back(driver);
    }
  }

  return preferred;
}

std::string escape_rml(std::string_view text) {
  std::string escaped;
  escaped.reserve(text.size());

  for (const char character : text) {
    switch (character) {
      case '&':
        escaped += "&amp;";
        break;
      case '<':
        escaped += "&lt;";
        break;
      case '>':
        escaped += "&gt;";
        break;
      case '"':
        escaped += "&quot;";
        break;
      case '\'':
        escaped += "&apos;";
        break;
      default:
        escaped.push_back(character);
        break;
    }
  }

  return escaped;
}

std::string css_rgba_from_hex(const std::string& hex, float alpha) {
  if (hex.size() != 7 || hex[0] != '#') {
    return "rgba(0, 0, 0, " + std::to_string(alpha) + ")";
  }

  const auto to_component = [&](std::size_t offset) {
    return std::stoi(hex.substr(offset, 2), nullptr, 16);
  };

  std::ostringstream stream;
  stream << "rgba(" << to_component(1) << ", " << to_component(3) << ", " << to_component(5) << ", " << alpha << ")";
  return stream.str();
}

std::string join_text_lines(const std::vector<std::string>& lines, std::string_view separator = " ") {
  std::ostringstream stream;

  for (std::size_t index = 0; index < lines.size(); ++index) {
    if (index > 0) {
      stream << separator;
    }
    stream << lines[index];
  }

  return stream.str();
}

std::filesystem::path resolve_vendor_font_path(const app::AppPaths& paths, std::string_view filename) {
  return paths.resource_root.parent_path() / "third_party" / "rmlui" / "Samples" / "assets" / filename;
}

std::filesystem::path resolve_project_font_path(const app::AppPaths& paths, std::string_view filename) {
  return paths.resource_root / "fonts" / filename;
}

std::string spinner_frame(Uint32 ticks) {
  static constexpr std::array<std::string_view, 16> frames = {
      "[    ]",
      "[=   ]",
      "[==  ]",
      "[=== ]",
      "[====]",
      "[ ===]",
      "[  ==]",
      "[   =]",
      "[    ]",
      "[   =]",
      "[  ==]",
      "[ ===]",
      "[====]",
      "[=== ]",
      "[==  ]",
      "[=   ]",
  };

  const std::size_t index = (ticks / 120) % frames.size();
  return std::string(frames[index]);
}

class SdlSystemInterface final : public Rml::SystemInterface {
 public:
  explicit SdlSystemInterface(SDL_Window* window)
      : window_(window),
        start_counter_(SDL_GetPerformanceCounter()),
        frequency_(static_cast<double>(SDL_GetPerformanceFrequency())) {}

  double GetElapsedTime() override {
    return static_cast<double>(SDL_GetPerformanceCounter() - start_counter_) / frequency_;
  }

  void SetMouseCursor(const Rml::String&) override {}

  void SetClipboardText(const Rml::String& text) override { SDL_SetClipboardText(text.c_str()); }

  void GetClipboardText(Rml::String& text) override {
    char* clipboard = SDL_GetClipboardText();
    if (!clipboard) {
      text.clear();
      return;
    }

    text = clipboard;
    SDL_free(clipboard);
  }

  void ActivateKeyboard(Rml::Vector2f, float) override { SDL_StartTextInput(); }

  void DeactivateKeyboard() override { SDL_StopTextInput(); }

 private:
  SDL_Window* window_ = nullptr;
  Uint64 start_counter_ = 0;
  double frequency_ = 1.0;
};

class SdlRenderInterface final : public Rml::RenderInterface {
 public:
  explicit SdlRenderInterface(SDL_Renderer* renderer) : renderer_(renderer) {
    blend_mode_ = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ONE,
        SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ONE,
        SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        SDL_BLENDOPERATION_ADD);
  }

  void BeginFrame() {
    SDL_RenderSetViewport(renderer_, nullptr);
    SDL_RenderSetClipRect(renderer_, nullptr);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    SDL_SetRenderDrawBlendMode(renderer_, blend_mode_);
  }

  Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override {
    auto geometry = std::make_unique<Geometry>();
    geometry->vertices.assign(vertices.begin(), vertices.end());
    geometry->indices.assign(indices.begin(), indices.end());
    return reinterpret_cast<Rml::CompiledGeometryHandle>(geometry.release());
  }

  void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override {
    delete reinterpret_cast<Geometry*>(geometry);
  }

  void RenderGeometry(Rml::CompiledGeometryHandle handle, Rml::Vector2f translation, Rml::TextureHandle texture) override {
    const auto* geometry = reinterpret_cast<const Geometry*>(handle);
    if (!geometry) {
      return;
    }

    std::vector<SDL_Vertex> sdl_vertices;
    sdl_vertices.reserve(geometry->vertices.size());

    for (const auto& vertex : geometry->vertices) {
      SDL_Vertex sdl_vertex{};
      sdl_vertex.position = SDL_FPoint{vertex.position.x + translation.x, vertex.position.y + translation.y};
      sdl_vertex.tex_coord = SDL_FPoint{vertex.tex_coord.x, vertex.tex_coord.y};
      sdl_vertex.color = SDL_Color{vertex.colour.red, vertex.colour.green, vertex.colour.blue, vertex.colour.alpha};
      sdl_vertices.push_back(sdl_vertex);
    }

    SDL_RenderGeometry(
        renderer_,
        reinterpret_cast<SDL_Texture*>(texture),
        sdl_vertices.data(),
        static_cast<int>(sdl_vertices.size()),
        geometry->indices.data(),
        static_cast<int>(geometry->indices.size()));
  }

  Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override {
    return {};
  }

  Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override {
    if (source.empty()) {
      return {};
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(
        const_cast<Rml::byte*>(source.data()),
        source_dimensions.x,
        source_dimensions.y,
        32,
        source_dimensions.x * 4,
        SDL_PIXELFORMAT_RGBA32);
    if (!surface) {
      return {};
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
      return {};
    }

    SDL_SetTextureBlendMode(texture, blend_mode_);
    return reinterpret_cast<Rml::TextureHandle>(texture);
  }

  void ReleaseTexture(Rml::TextureHandle texture_handle) override {
    SDL_DestroyTexture(reinterpret_cast<SDL_Texture*>(texture_handle));
  }

  void EnableScissorRegion(bool enable) override {
    scissor_enabled_ = enable;
    SDL_RenderSetClipRect(renderer_, enable ? &scissor_rect_ : nullptr);
  }

  void SetScissorRegion(Rml::Rectanglei region) override {
    scissor_rect_.x = region.Left();
    scissor_rect_.y = region.Top();
    scissor_rect_.w = region.Width();
    scissor_rect_.h = region.Height();

    if (scissor_enabled_) {
      SDL_RenderSetClipRect(renderer_, &scissor_rect_);
    }
  }

 private:
  struct Geometry {
    std::vector<Rml::Vertex> vertices;
    std::vector<int> indices;
  };

  SDL_Renderer* renderer_ = nullptr;
  SDL_BlendMode blend_mode_ = SDL_BLENDMODE_BLEND;
  SDL_Rect scissor_rect_{};
  bool scissor_enabled_ = false;
};

struct RenderedDocument {
  std::string rml;
  std::string focus_element_id;
};

bool is_focused(const std::optional<app::FocusState>& focus_state, app::FocusArea area, std::size_t index) {
  return focus_state.has_value() && focus_state->area == area && focus_state->item_index == index;
}

void append_panel(
    std::ostringstream& stream,
    std::string_view tag,
    std::string_view id,
    std::string_view content,
    std::string_view extra_style,
    bool focused,
    const theme::Theme& theme) {
  stream << "<" << tag << " id=\"" << id << "\" style=\""
         << "display:block;"
         << "padding:" << theme.spacing_unit_px << "px;"
         << "margin-bottom:" << (theme.spacing_unit_px / 2) << "px;"
         << "background-color:" << (focused ? theme.focus_hex : theme.panel_hex) << ";"
         << "color:#ffffff;"
         << "border-radius:10px;"
         << "box-sizing:border-box;"
         << extra_style << "\">"
         << content
         << "</" << tag << ">";
}

RenderedDocument build_document(
    const ScreenModel& model,
    const theme::Theme& theme,
    const BackendInfo& backend_info,
    std::optional<app::FocusState> focus_state,
    app::RuntimeMode runtime_mode,
    int viewport_width_px) {
  const bool preview_mode = runtime_mode == app::RuntimeMode::Preview;
  const int welcome_content_width_px = std::max(viewport_width_px - (theme.spacing_unit_px * 8), 320);
  std::ostringstream stream;
  std::string focus_element_id;

  const bool force_vertical_scrollbar = model.screen_id == "welcome" || model.screen_id == "wifi-list";

  stream << "<rml><head><title>" << escape_rml(model.title) << "</title>"
         << "<style>"
         << "scrollbarvertical{width:22px;margin-left:8px;background-color:#ffffff;}"
         << "scrollbarvertical slidertrack{background-color:#f2f2f2;}"
         << "scrollbarvertical sliderbar{background-color:#9000ff;min-height:64px;margin:2px;}"
         << "scrollbarvertical sliderbar:hover,scrollbarvertical sliderbar:active{background-color:#1a1a1a;}"
         << "scrollbarhorizontal{height:22px;background-color:#ffffff;}"
         << "scrollbarhorizontal slidertrack{background-color:#f2f2f2;}"
         << "scrollbarhorizontal sliderbar{background-color:#9000ff;min-width:64px;margin:2px;}"
         << "</style></head>"
         << "<body style=\"margin:0;padding:0;width:100%;height:100%;"
         << "background-color:" << theme.background_hex << ";"
         << "color:" << theme.text_hex << ";"
         << "font-family:&quot;JetBrains Mono&quot;;"
         << "font-size:" << theme.typography.body_text_px << "px;\">"
         << "<div style=\"position:relative;display:flex;flex-direction:column;"
         << "width:100%;height:100%;padding:" << (theme.spacing_unit_px * 2) << "px;"
         << "box-sizing:border-box;gap:" << theme.spacing_unit_px << "px;\">"
         << "<div style=\"font-size:" << theme.typography.screen_title_px << "px;font-weight:bold;\">"
         << escape_rml(model.title)
         << "</div>"
         << "<div style=\"display:flex;flex-direction:column;flex:1;min-height:0;gap:" << theme.spacing_unit_px << "px;\">";

  if (model.screen_id == "status" && model.body_lines.size() >= 2) {
    stream << "<div style=\"display:flex;flex-direction:column;align-items:center;justify-content:center;"
           << "flex:1;min-height:0;padding:" << theme.spacing_unit_px << "px;\">"
           << "<div style=\"font-size:" << theme.typography.pin_label_px << "px;text-align:center;margin-bottom:" << theme.spacing_unit_px << "px;\">"
           << escape_rml(model.body_lines[0])
           << "</div>"
           << "<div style=\"font-size:" << theme.typography.pin_digits_px << "px;font-weight:bold;line-height:1;text-align:center;\">"
           << escape_rml(model.body_lines[1])
           << "</div>"
           << "</div>";
  } else if (model.show_spinner) {
    stream << "<div style=\"display:flex;flex-direction:column;align-items:center;justify-content:center;"
           << "flex:1;min-height:0;gap:" << theme.spacing_unit_px << "px;\">";
    for (const auto& line : model.body_lines) {
      stream << "<div style=\"line-height:1.3;white-space:pre-wrap;text-align:center;\">"
             << escape_rml(line)
             << "</div>";
    }
    stream << "<div style=\"font-size:" << theme.typography.dialog_title_px
           << "px;font-weight:bold;letter-spacing:2px;text-align:center;white-space:pre;display:inline-block;\">"
           << escape_rml(spinner_frame(SDL_GetTicks()))
           << "</div>"
           << "</div>";
  } else if (!model.body_lines.empty()) {
    stream << "<div style=\"display:flex;flex-direction:column;gap:" << (theme.spacing_unit_px / 2) << "px;\">";
    for (const auto& line : model.body_lines) {
      stream << "<div style=\"line-height:1.3;white-space:pre-wrap;\">"
             << escape_rml(line)
             << "</div>";
    }
    stream << "</div>";
  }

  if (!model.list.title.empty()) {
    stream << "<div style=\"font-size:" << theme.typography.helper_text_px << "px;font-weight:bold;\">"
           << escape_rml(model.list.title)
           << "</div>";
  }

  if (model.screen_id == "welcome" && (!model.list.items.empty() || model.list.empty_state_text.has_value())) {
    const bool panel_focused = focus_state.has_value() && focus_state->area == app::FocusArea::List;
    stream << "<div style=\"display:block;flex:1;min-height:0;min-width:0;width:100%;box-sizing:border-box;"
            << "padding:" << theme.focus_outline_px << "px;"
            << "background-color:" << (panel_focused ? theme.focus_hex : theme.text_hex) << ";\">"
           << "<div id=\"welcome-scroll\" style=\"display:block;height:100%;min-width:0;overflow-y:scroll;"
           << "width:100%;box-sizing:border-box;padding:" << theme.spacing_unit_px << "px;"
           << "padding-right:" << (theme.spacing_unit_px + 8) << "px;"
           << "background-color:" << theme.panel_hex << ";color:#ffffff;\">"
           << "<div style=\"display:block;min-width:0;width:" << welcome_content_width_px << "px;\">";

    if (model.list.items.empty() && model.list.empty_state_text.has_value()) {
      stream << "<div id=\"welcome-empty\" style=\"font-size:" << theme.typography.body_text_px
             << "px;line-height:1.35;white-space:pre-wrap;\">"
             << escape_rml(*model.list.empty_state_text)
             << "</div>";
    }

    for (std::size_t index = 0; index < model.list.items.size(); ++index) {
      const auto& item = model.list.items[index];
      const std::string element_id = "welcome-paragraph-" + std::to_string(index);
      const bool focused = is_focused(focus_state, app::FocusArea::List, index);

      if (focused) {
        focus_element_id = element_id;
      }

      stream << "<div id=\"" << element_id << "\" style=\"display:block;width:" << welcome_content_width_px << "px;min-width:0;box-sizing:border-box;"
             << "padding:" << (theme.spacing_unit_px / 2) << "px 0;"
             << "margin-bottom:" << (theme.spacing_unit_px / 2) << "px;"
              << "font-size:" << theme.typography.body_text_px << "px;"
              << "line-height:1.35;white-space:normal;word-break:normal;\">";

      stream << "<div style=\"display:block;width:" << welcome_content_width_px << "px;min-width:0;white-space:normal;word-break:normal;\">"
             << escape_rml(item.primary_text)
             << "</div>";

      if (!item.secondary_text.empty()) {
        stream << "<div style=\"display:block;width:" << welcome_content_width_px << "px;min-width:0;font-size:" << theme.typography.helper_text_px
                << "px;margin-top:" << (theme.spacing_unit_px / 2)
                << "px;line-height:1.3;white-space:normal;word-break:normal;\">"
                << escape_rml(item.secondary_text)
               << "</div>";
      }

      stream << "</div>";
    }

    stream << "</div></div></div>";
  } else if (!model.list.items.empty() || model.list.empty_state_text.has_value()) {
    stream << "<div id=\"list-scroll\" style=\"display:block;flex:1;min-height:0;overflow-y:"
           << (force_vertical_scrollbar ? "scroll" : "auto") << ";"
           << "padding-right:" << (theme.spacing_unit_px / 2) << "px;\">";

    if (model.list.items.empty() && model.list.empty_state_text.has_value()) {
      append_panel(
          stream,
          "div",
          "empty-state",
          escape_rml(*model.list.empty_state_text),
          "white-space:pre-wrap;",
          false,
          theme);
    }

    for (std::size_t index = 0; index < model.list.items.size(); ++index) {
      const auto& item = model.list.items[index];
      const std::string element_id = "list-item-" + std::to_string(index);
      const bool focused = is_focused(focus_state, app::FocusArea::List, index);

      if (focused) {
        focus_element_id = element_id;
      }

      std::ostringstream content;
      content << "<div style=\"font-size:" << theme.typography.body_text_px << "px;font-weight:bold;\">"
              << escape_rml(item.primary_text)
              << "</div>";
      if (!item.secondary_text.empty()) {
        content << "<div style=\"font-size:" << theme.typography.helper_text_px << "px;margin-top:"
                << (theme.spacing_unit_px / 4) << "px;white-space:pre-wrap;\">"
                << escape_rml(item.secondary_text)
                << "</div>";
      }

      append_panel(stream, "button", element_id, content.str(), "width:100%;text-align:left;", focused, theme);
    }

    stream << "</div>";
  }

  if (!model.actions.items.empty()) {
    stream << "<div style=\"display:flex;flex-direction:row;gap:" << theme.spacing_unit_px << "px;flex-wrap:wrap;\">";
    for (std::size_t index = 0; index < model.actions.items.size(); ++index) {
      const auto& action = model.actions.items[index];
      const std::string element_id = "action-item-" + std::to_string(index);
      const bool focused = is_focused(focus_state, app::FocusArea::ActionRow, index);

      if (focused) {
        focus_element_id = element_id;
      }

      append_panel(
          stream,
          "button",
          element_id,
          escape_rml(action.label),
          "min-width:220px;text-align:center;",
          focused,
          theme);
    }
    stream << "</div>";
  }

  stream << "</div>";

  if (model.dialog.has_value()) {
    stream << "<div style=\"position:absolute;inset:0;background-color:"
           << css_rgba_from_hex(theme.panel_hex, 0.65f)
           << ";\">"
           << "<div style=\"position:absolute;left:12%;top:18%;width:76%;"
           << "background-color:#ffffff;color:" << theme.text_hex << ";"
           << "box-shadow:0 0 0 " << theme.focus_outline_px << "px " << theme.focus_hex << ";"
           << "border-radius:14px;padding:" << (theme.spacing_unit_px * 2) << "px;box-sizing:border-box;\">"
           << "<div style=\"font-size:" << theme.typography.dialog_title_px << "px;font-weight:bold;margin-bottom:" << theme.spacing_unit_px << "px;\">"
           << escape_rml(model.dialog->title)
           << "</div>";

    for (const auto& line : model.dialog->body_lines) {
      stream << "<div style=\"font-size:" << theme.typography.body_text_px << "px;line-height:1.3;margin-bottom:"
             << (theme.spacing_unit_px / 2) << "px;white-space:pre-wrap;\">"
             << escape_rml(line)
             << "</div>";
    }

    stream << "<div style=\"display:flex;flex-direction:row;gap:" << theme.spacing_unit_px << "px;flex-wrap:wrap;margin-top:" << theme.spacing_unit_px << "px;\">";
    for (std::size_t index = 0; index < model.dialog->actions.size(); ++index) {
      const auto& action = model.dialog->actions[index];
      const std::string element_id = "dialog-item-" + std::to_string(index);
      const bool focused = is_focused(focus_state, app::FocusArea::Dialog, index);

      if (focused) {
        focus_element_id = element_id;
      }

      append_panel(
          stream,
          "button",
          element_id,
          escape_rml(action.label),
          "min-width:220px;text-align:center;",
          focused,
          theme);
    }
    stream << "</div></div></div>";
  }

  stream << "</div></body></rml>";
  return RenderedDocument{.rml = stream.str(), .focus_element_id = focus_element_id};
}

app::InputCommand map_key(SDL_Keycode keycode) {
  switch (keycode) {
    case SDLK_UP:
    case SDLK_w:
      return app::InputCommand::Up;
    case SDLK_DOWN:
    case SDLK_s:
      return app::InputCommand::Down;
    case SDLK_LEFT:
    case SDLK_a:
      return app::InputCommand::Left;
    case SDLK_RIGHT:
    case SDLK_d:
      return app::InputCommand::Right;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
    case SDLK_SPACE:
    case SDLK_e:
      return app::InputCommand::Confirm;
    case SDLK_BACKSPACE:
    case SDLK_b:
      return app::InputCommand::Back;
    case SDLK_ESCAPE:
    case SDLK_q:
      return app::InputCommand::Exit;
    default:
      return app::InputCommand::None;
  }
}

}  // namespace

struct RmlUiRenderer::Impl {
  explicit Impl(app::AppPaths in_paths, app::RuntimeMode in_runtime_mode)
      : paths(std::move(in_paths)), runtime_mode(in_runtime_mode) {}

  ~Impl() {
    if (document != nullptr) {
      document->Close();
      document = nullptr;
    }

    if (context != nullptr) {
      Rml::RemoveContext("rook-ui");
      context = nullptr;
    }

    if (rml_initialized) {
      Rml::Shutdown();
      rml_initialized = false;
    }

    for (auto& [_, controller] : controllers) {
      if (controller != nullptr) {
        SDL_GameControllerClose(controller);
      }
    }
    controllers.clear();

    if (renderer != nullptr) {
      SDL_DestroyRenderer(renderer);
      renderer = nullptr;
    }

    if (window != nullptr) {
      SDL_DestroyWindow(window);
      window = nullptr;
    }

    if (sdl_initialized) {
      SDL_Quit();
      sdl_initialized = false;
    }
  }

  bool initialize() {
    const char* forced_driver = std::getenv("SDL_VIDEODRIVER");
    const std::vector<std::string> available_drivers = available_video_drivers();
    std::vector<std::string> attempted_drivers;
    std::vector<std::string> attempt_errors;

    if (forced_driver != nullptr && forced_driver[0] != '\0') {
      attempted_drivers.emplace_back(forced_driver);
    } else {
      attempted_drivers = preferred_video_drivers(runtime_mode, available_drivers);
    }

    if (attempted_drivers.empty()) {
      std::cerr << "No SDL video drivers are available in this environment.\n";
      return false;
    }

    const auto try_driver = [&](const std::string& driver) {
      reset_sdl_runtime();
      ::setenv("SDL_VIDEODRIVER", driver.c_str(), 1);

      if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
        attempt_errors.push_back(driver + ": SDL initialisation failed: " + SDL_GetError());
        return false;
      }
      sdl_initialized = true;

      const bool is_kmsdrm = driver == "kmsdrm";
      const Uint32 window_flags = SDL_WINDOW_ALLOW_HIGHDPI |
                                  ((runtime_mode == app::RuntimeMode::Preview && !is_kmsdrm) ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN);
      const int width = runtime_mode == app::RuntimeMode::Preview ? kPreviewWindowWidth : kNormalWindowWidth;
      const int height = runtime_mode == app::RuntimeMode::Preview ? kPreviewWindowHeight : kNormalWindowHeight;
      const char* title = runtime_mode == app::RuntimeMode::Preview ? "RooK Console UI Preview" : "RooK Service";

      window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
      if (window == nullptr) {
        attempt_errors.push_back(driver + ": SDL window creation failed: " + SDL_GetError());
        return false;
      }

      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
      }
      if (renderer == nullptr) {
        attempt_errors.push_back(driver + ": SDL renderer creation failed: " + SDL_GetError());
        return false;
      }

      active_video_driver = driver;
      std::cerr << "SDL video driver: " << active_video_driver << '\n';
      return true;
    };

    bool sdl_ready = false;
    for (const auto& driver : attempted_drivers) {
      if (try_driver(driver)) {
        sdl_ready = true;
        break;
      }
    }

    if (!sdl_ready) {
      reset_sdl_runtime();
      std::cerr << "SDL graphical startup failed. Attempted video drivers:";
      for (const auto& driver : attempted_drivers) {
        std::cerr << ' ' << driver;
      }
      std::cerr << '\n';
      for (const auto& error : attempt_errors) {
        std::cerr << error << '\n';
      }
      return false;
    }

    system_interface = std::make_unique<SdlSystemInterface>(window);
    render_interface = std::make_unique<SdlRenderInterface>(renderer);

    Rml::SetSystemInterface(system_interface.get());
    Rml::SetRenderInterface(render_interface.get());

    if (!Rml::Initialise()) {
      std::cerr << "RmlUi initialisation failed.\n";
      return false;
    }
    rml_initialized = true;

    int output_width = 0;
    int output_height = 0;
    SDL_GetRendererOutputSize(renderer, &output_width, &output_height);
    context = Rml::CreateContext("rook-ui", Rml::Vector2i(output_width, output_height));
    if (context == nullptr) {
      std::cerr << "RmlUi context creation failed.\n";
      return false;
    }

    if (!load_fonts()) {
      std::cerr << "RmlUi fonts could not be loaded from vendored assets.\n";
      return false;
    }

    SDL_GameControllerEventState(SDL_ENABLE);
    open_attached_controllers();
    return true;
  }

  void render_screen(
      const ScreenModel& model,
      const theme::Theme& theme,
      const BackendInfo& backend_info,
      std::optional<app::FocusState> focus_state) {
    if (context == nullptr || render_interface == nullptr || renderer == nullptr) {
      return;
    }

    int output_width = 0;
    int output_height = 0;
    SDL_GetRendererOutputSize(renderer, &output_width, &output_height);
    const auto document_data = build_document(model, theme, backend_info, focus_state, runtime_mode, output_width);

    if (document != nullptr) {
      document->Close();
      document = nullptr;
    }

    SDL_SetWindowTitle(window, model.title.c_str());
    document = context->LoadDocumentFromMemory(document_data.rml);
    if (document == nullptr) {
      std::cerr << "RmlUi failed to create document for screen " << model.screen_id << ".\n";
      return;
    }

    document->Show();
    if (!document_data.focus_element_id.empty()) {
      if (Rml::Element* focused = document->GetElementById(document_data.focus_element_id); focused != nullptr) {
        focused->ScrollIntoView(false);
      }
    }

    context->Update();
    render_interface->BeginFrame();
    context->Render();
    SDL_RenderPresent(renderer);
  }

  app::InputCommand read_command(bool animate) {
    SDL_Event event{};

    if (animate) {
      if (SDL_WaitEventTimeout(&event, 120) == 0) {
        return app::InputCommand::Tick;
      }

      if (const auto command = handle_event(event); command != app::InputCommand::None) {
        return command;
      }

      return app::InputCommand::None;
    }

    while (SDL_WaitEvent(&event) == 1) {
      if (const auto command = handle_event(event); command != app::InputCommand::None) {
        return command;
      }
    }

    return app::InputCommand::Exit;
  }

 private:
  bool load_fonts() {
    const std::array<std::string_view, 4> faces = {
        "JetBrainsMono-Regular.ttf",
        "JetBrainsMono-Italic.ttf",
        "JetBrainsMono-Bold.ttf",
        "JetBrainsMono-BoldItalic.ttf",
    };

    for (const auto face : faces) {
      if (!Rml::LoadFontFace(resolve_project_font_path(paths, face).string())) {
        return false;
      }
    }

    Rml::LoadFontFace(resolve_vendor_font_path(paths, "NotoEmoji-Regular.ttf").string(), true);
    return true;
  }

  void open_attached_controllers() {
    const int joystick_count = SDL_NumJoysticks();
    for (int index = 0; index < joystick_count; ++index) {
      open_controller(index);
    }
  }

  void open_controller(int device_index) {
    if (!SDL_IsGameController(device_index)) {
      return;
    }

    SDL_GameController* controller = SDL_GameControllerOpen(device_index);
    if (controller == nullptr) {
      return;
    }

    SDL_Joystick* joystick = SDL_GameControllerGetJoystick(controller);
    controllers.insert_or_assign(SDL_JoystickInstanceID(joystick), controller);
  }

  void close_controller(SDL_JoystickID instance_id) {
    const auto iterator = controllers.find(instance_id);
    if (iterator == controllers.end()) {
      return;
    }

    SDL_GameControllerClose(iterator->second);
    controllers.erase(iterator);
  }

  void resize_context() {
    if (context == nullptr || renderer == nullptr) {
      return;
    }

    int output_width = 0;
    int output_height = 0;
    SDL_GetRendererOutputSize(renderer, &output_width, &output_height);
    context->SetDimensions(Rml::Vector2i(output_width, output_height));
  }

  app::InputCommand map_controller_button(const SDL_ControllerButtonEvent& event) const {
    switch (event.button) {
      case SDL_CONTROLLER_BUTTON_DPAD_UP:
      case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        return app::InputCommand::Up;
      case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
      case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        return app::InputCommand::Down;
      case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        return app::InputCommand::Left;
      case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        return app::InputCommand::Right;
      case SDL_CONTROLLER_BUTTON_A:
      case SDL_CONTROLLER_BUTTON_START:
        return app::InputCommand::Confirm;
      case SDL_CONTROLLER_BUTTON_B:
      case SDL_CONTROLLER_BUTTON_BACK:
        return app::InputCommand::Back;
      case SDL_CONTROLLER_BUTTON_GUIDE:
        return app::InputCommand::Exit;
      default:
        return app::InputCommand::None;
    }
  }

  app::InputCommand handle_axis_motion(const SDL_ControllerAxisEvent& event) {
    auto evaluate_axis = [](Sint16 value, bool& negative_latched, bool& positive_latched, app::InputCommand negative, app::InputCommand positive) {
      if (value <= -kAxisPressThreshold && !negative_latched) {
        negative_latched = true;
        positive_latched = false;
        return negative;
      }

      if (value >= kAxisPressThreshold && !positive_latched) {
        positive_latched = true;
        negative_latched = false;
        return positive;
      }

      if (value > -kAxisReleaseThreshold && value < kAxisReleaseThreshold) {
        negative_latched = false;
        positive_latched = false;
      }

      return app::InputCommand::None;
    };

    switch (event.axis) {
      case SDL_CONTROLLER_AXIS_LEFTX:
        return evaluate_axis(event.value, left_negative_latched, left_positive_latched, app::InputCommand::Left, app::InputCommand::Right);
      case SDL_CONTROLLER_AXIS_LEFTY:
        return evaluate_axis(event.value, up_negative_latched, up_positive_latched, app::InputCommand::Up, app::InputCommand::Down);
      default:
        return app::InputCommand::None;
    }
  }

  app::InputCommand handle_event(const SDL_Event& event) {
    switch (event.type) {
      case SDL_QUIT:
        return app::InputCommand::Exit;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_EXPOSED) {
          resize_context();
        } else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
          return app::InputCommand::Exit;
        }
        return app::InputCommand::None;
      case SDL_KEYDOWN:
        if (event.key.repeat != 0) {
          return app::InputCommand::None;
        }
        return map_key(event.key.keysym.sym);
      case SDL_CONTROLLERDEVICEADDED:
        open_controller(event.cdevice.which);
        return app::InputCommand::None;
      case SDL_CONTROLLERDEVICEREMOVED:
        close_controller(event.cdevice.which);
        return app::InputCommand::None;
      case SDL_CONTROLLERBUTTONDOWN:
        return map_controller_button(event.cbutton);
      case SDL_CONTROLLERAXISMOTION:
        return handle_axis_motion(event.caxis);
      default:
        return app::InputCommand::None;
    }
  }

  app::AppPaths paths;
  app::RuntimeMode runtime_mode = app::RuntimeMode::Normal;
  std::string active_video_driver;
  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  std::unique_ptr<SdlSystemInterface> system_interface;
  std::unique_ptr<SdlRenderInterface> render_interface;
  Rml::Context* context = nullptr;
  Rml::ElementDocument* document = nullptr;
  std::unordered_map<SDL_JoystickID, SDL_GameController*> controllers;
  bool sdl_initialized = false;
  bool rml_initialized = false;
  bool left_negative_latched = false;
  bool left_positive_latched = false;
  bool up_negative_latched = false;
  bool up_positive_latched = false;

  void reset_sdl_runtime() {
    for (auto& [_, controller] : controllers) {
      if (controller != nullptr) {
        SDL_GameControllerClose(controller);
      }
    }
    controllers.clear();

    if (renderer != nullptr) {
      SDL_DestroyRenderer(renderer);
      renderer = nullptr;
    }

    if (window != nullptr) {
      SDL_DestroyWindow(window);
      window = nullptr;
    }

    if (sdl_initialized) {
      SDL_Quit();
      sdl_initialized = false;
    }
  }
};

RmlUiRenderer::RmlUiRenderer(app::AppPaths paths, app::RuntimeMode runtime_mode)
    : impl_(std::make_unique<Impl>(std::move(paths), runtime_mode)) {}

RmlUiRenderer::~RmlUiRenderer() = default;

RmlUiRenderer::RmlUiRenderer(RmlUiRenderer&&) noexcept = default;
RmlUiRenderer& RmlUiRenderer::operator=(RmlUiRenderer&&) noexcept = default;

bool RmlUiRenderer::initialize() { return impl_->initialize(); }

void RmlUiRenderer::render_screen(
    const ScreenModel& model,
    const theme::Theme& theme,
    const BackendInfo& backend_info,
    std::optional<app::FocusState> focus_state) {
  impl_->render_screen(model, theme, backend_info, focus_state);
}

app::InputCommand RmlUiRenderer::read_command(bool animate) { return impl_->read_command(animate); }

}  // namespace rook::ui::render

#else

namespace rook::ui::render {

RmlUiRenderer::RmlUiRenderer(app::AppPaths, app::RuntimeMode) {}
RmlUiRenderer::~RmlUiRenderer() = default;
RmlUiRenderer::RmlUiRenderer(RmlUiRenderer&&) noexcept = default;
RmlUiRenderer& RmlUiRenderer::operator=(RmlUiRenderer&&) noexcept = default;
bool RmlUiRenderer::initialize() { return false; }
void RmlUiRenderer::render_screen(const ScreenModel&, const theme::Theme&, const BackendInfo&, std::optional<app::FocusState>) {}
app::InputCommand RmlUiRenderer::read_command(bool) { return app::InputCommand::Exit; }

}  // namespace rook::ui::render

#endif
