#include "app/UiSettings.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "third_party/nlohmann/json.hpp"

namespace rook::ui::app {

UiSettingsStore::UiSettingsStore(std::filesystem::path settings_file)
    : settings_file_(std::move(settings_file)) {}

UiSettings UiSettingsStore::load() const {
  if (!std::filesystem::exists(settings_file_)) {
    return UiSettings{};
  }

  std::ifstream file(settings_file_);
  if (!file.is_open()) {
    throw std::runtime_error("settings file could not be opened");
  }

  nlohmann::json document = nlohmann::json::parse(file);
  return UiSettings{
      .hide_welcome = document.value("hideWelcome", false),
  };
}

void UiSettingsStore::save(const UiSettings& settings) const {
  std::filesystem::create_directories(settings_file_.parent_path());

  std::ofstream file(settings_file_, std::ios::trunc);
  if (!file.is_open()) {
    throw std::runtime_error("settings file could not be written");
  }

  nlohmann::json document = {
      {"hideWelcome", settings.hide_welcome},
  };

  file << document.dump(2) << '\n';
}

}  // namespace rook::ui::app
