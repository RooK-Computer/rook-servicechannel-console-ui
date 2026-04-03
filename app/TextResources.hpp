#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace rook::ui::app {

std::vector<std::string> load_welcome_text_lines(const std::filesystem::path& resource_root);

}  // namespace rook::ui::app
