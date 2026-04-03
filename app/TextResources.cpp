#include "app/TextResources.hpp"

#include <fstream>

namespace rook::ui::app {

std::vector<std::string> load_welcome_text_lines(const std::filesystem::path& resource_root) {
  const auto path = resource_root / "text" / "welcome.txt";
  std::ifstream file(path);
  if (!file.is_open()) {
    return {
        "Mit RooK Service kann eine Support-Sitzung fuer diese Konsole gestartet werden.",
        "Die Verbindung wird nur lokal an diesem Geraet gestartet und zeigt danach eine Service-PIN an.",
        "Fuer den Aufbau kann zunaechst eine WLAN-Verbindung und danach eine sichere Verbindung eingerichtet werden.",
    };
  }

  std::vector<std::string> lines;
  for (std::string line; std::getline(file, line);) {
    if (!line.empty()) {
      lines.push_back(line);
    }
  }

  if (lines.empty()) {
    lines.push_back("RooK Service");
  }

  return lines;
}

}  // namespace rook::ui::app
