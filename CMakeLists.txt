cmake_minimum_required(VERSION 3.20)

project(rook-ui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ROOK_UI_WITH_SDL2 "Link against SDL2 when available" ON)
option(ROOK_UI_WITH_RMLUI "Build with vendored RmlUi when available" ON)

set(ROOK_UI_RMLUI_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/rmlui"
    CACHE PATH
    "Path to the vendored RmlUi source tree")

if(ROOK_UI_WITH_SDL2)
  find_package(SDL2 CONFIG QUIET)
endif()

if(ROOK_UI_WITH_RMLUI)
  if(EXISTS "${ROOK_UI_RMLUI_SOURCE_DIR}/CMakeLists.txt")
    set(RMLUI_SAMPLES OFF CACHE BOOL "Disable vendored RmlUi samples" FORCE)
    set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "Disable vendored RmlUi Lua bindings" FORCE)
    set(RMLUI_PRECOMPILED_HEADERS OFF CACHE BOOL "Disable vendored RmlUi precompiled headers" FORCE)
    set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "Build vendored RmlUi with FreeType font support" FORCE)
    add_subdirectory("${ROOK_UI_RMLUI_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/third_party/rmlui" EXCLUDE_FROM_ALL)
  else()
    message(FATAL_ERROR
      "ROOK_UI_WITH_RMLUI=ON, but vendored RmlUi was not found at '${ROOK_UI_RMLUI_SOURCE_DIR}'. "
      "Run 'make deps' or 'git submodule update --init --recursive third_party/rmlui'.")
  endif()
endif()

add_executable(rook-ui
  app/main.cpp
  app/Application.cpp
  app/AppPaths.cpp
  app/CommandLine.cpp
  app/FocusEngine.cpp
  app/Intent.cpp
  app/NavigationSession.cpp
  app/PreviewRegistry.cpp
  app/ScreenRegistry.cpp
  app/TerminalInput.cpp
  app/TextResources.cpp
  render/BackendInfo.cpp
  render/RmlUiRenderer.cpp
  render/TerminalRenderer.cpp
  screens/KeyboardScreen.cpp
  screens/PlaceholderScreen.cpp
  screens/StatusScreen.cpp
  screens/VpnErrorScreen.cpp
  screens/VpnWaitScreen.cpp
  screens/WelcomeScreen.cpp
  screens/WifiErrorScreen.cpp
  screens/WifiListScreen.cpp
  screens/WifiWaitScreen.cpp
  theme/Theme.cpp
)

target_include_directories(rook-ui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_features(rook-ui PRIVATE cxx_std_20)

if(TARGET SDL2::SDL2)
  target_link_libraries(rook-ui PRIVATE SDL2::SDL2)
  target_compile_definitions(rook-ui PRIVATE ROOK_UI_HAS_SDL2=1)
endif()

if(TARGET RmlUi::Core)
  target_link_libraries(rook-ui PRIVATE RmlUi::Core)
  target_compile_definitions(rook-ui PRIVATE ROOK_UI_HAS_RMLUI=1)
endif()

if(TARGET SDL2::SDL2 AND TARGET RmlUi::Core)
  target_compile_definitions(rook-ui PRIVATE RMLUI_SDL_VERSION_MAJOR=2)
endif()
