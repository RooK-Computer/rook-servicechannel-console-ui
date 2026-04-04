#pragma once

#include <filesystem>

namespace rook::ui::app {

struct UiSettings {
  bool hide_welcome = false;
};

class UiSettingsStore {
 public:
  explicit UiSettingsStore(std::filesystem::path settings_file);

  [[nodiscard]] UiSettings load() const;
  void save(const UiSettings& settings) const;

 private:
  std::filesystem::path settings_file_;
};

}  // namespace rook::ui::app
