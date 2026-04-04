#include "screens/WelcomeScreen.hpp"

#include "app/Intent.hpp"
#include "app/TextResources.hpp"

namespace rook::ui::screens {

WelcomeScreen::WelcomeScreen(app::AppPaths paths) : paths_(std::move(paths)) {}

std::string_view WelcomeScreen::id() const { return "welcome"; }

std::string_view WelcomeScreen::title() const { return "RooK Service"; }

render::ScreenModel WelcomeScreen::model(const ScreenContext& context) const {
  const auto welcome_lines = app::load_welcome_text_lines(paths_.resource_root);
  const auto continue_target = context.param_or("continue-target", "wifi-list");
  std::vector<components::ListItem> welcome_items;
  welcome_items.reserve(welcome_lines.size());

  for (std::size_t index = 0; index < welcome_lines.size(); ++index) {
    welcome_items.push_back(components::ListItem{
        .id = "welcome-text-" + std::to_string(index),
        .primary_text = welcome_lines[index],
    });
  }

  const bool hide_welcome = context.flag("hide-welcome");

  return render::ScreenModel{
      .screen_id = "welcome",
      .title = "RooK Service",
      .list = components::ListSection{
          .id = "welcome-body",
          .items = std::move(welcome_items),
      },
      .actions = components::ActionRow{
          .id = "welcome-actions",
          .items = {
               components::ActionItem{
                   .id = "welcome-hide",
                   .label = hide_welcome ? "[x] Beim naechsten Start nicht mehr anzeigen" : "[ ] Beim naechsten Start nicht mehr anzeigen",
                   .intent = app::navigate_to("welcome", {{"hide-welcome", hide_welcome ? "false" : "true"}}),
               },
               components::ActionItem{
                   .id = "welcome-continue",
                   .label = "Weiter",
                   .intent = app::navigate_to(continue_target),
               },
               components::ActionItem{
                   .id = "welcome-exit",
                   .label = "Beenden",
                   .intent = app::close_app(),
               },
           },
       },
   };
}

}  // namespace rook::ui::screens
