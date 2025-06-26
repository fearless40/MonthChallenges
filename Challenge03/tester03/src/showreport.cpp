#include "showreport.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "programoptions.hpp"
#include "virtualgames.hpp"
#include <atomic>
#include <format>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/dom/table.hpp>
#include <iterator>

#include <string>
#include <vector>
namespace ui {

struct AppTabs {
  std::vector<ftxui::Component> components;
  std::vector<std::string> labels;
  int selected;
  ftxui::ScreenInteractive screen{ftxui::ScreenInteractive::Fullscreen()};
  ftxui::Component header_c;

  ftxui::Component tabs_c;

  ftxui::Component container_c;

  void add_tab(std::string label, ftxui::Component comp) {
    components.push_back(comp);
    labels.push_back(label);
    container_c->Add(comp);
    tabs_c = ftxui::Toggle(labels, &selected);
    screen.ExitLoopClosure()();
    screen.Loop(ftxui::Container::Vertical({header_c, tabs_c, container_c}));
  }

  void remove_tab(std::string label) {
    auto it = std::ranges::find(labels, label);
    if (it != labels.end()) {
      auto distance = std::distance(labels.begin(), it);
      labels.erase(it);
      components.erase(components.begin() + distance);
      ftxui::ScreenInteractive::Active()->PostEvent(
          ftxui::Event::Custom); // force redraw
    }
  }
};

namespace global {
AppTabs tabs;

}
void AiButtonClick(AIID id) {}
// std::accumulate(games.begin(), games.end(),
//                            std::chrono::milliseconds(0),
//                            [](std::chrono::milliseconds const &time,
//                               VirtualGames const &value) {
//                              return time + value.global_stats().total_time;
//
ftxui::Component overview_tab(std::vector<VirtualGames> const &games) {
  using namespace ftxui;
  // auto header = hbox({text(std::format("Total time: {}", 5)), separator(),
  //                     text(std::format("Total AIs Run: {}", games.size()))});
  //
  // std::vector<std::string> ColHeaders = {"AI", "Average Guesses", "Won",
  // "Lost",
  //                                        "Repeats"};
  // std::vector<std::vector<std::string>> rows;
  // std::vector<Element> ai_details;
  Components ai_buttons;

  for (auto const &game : games) {
    std::string buttonlbl = std::format("AI {}", game.aiid());

    auto button = Button(buttonlbl, std::bind(AiButtonClick, game.aiid()),
                         ButtonOption::Animated(Color::Palette256::BlueViolet));
    auto button_row =
        Renderer(button, [avg_guess = game.global_stats().average_guess_count,
                          button]() {
          return hbox({button->Render(), separator(),
                       text(std::to_string(avg_guess)) | center | vcenter}) |
                 border;
        });
    ai_buttons.push_back(button_row);

    // rows.push_back({std::to_string(game.aiid()),

    //                       VirtualGames::EndingState::sunk_all_ships)),
    //                   std::to_string(game.global_stats().ending_state(
    //                       VirtualGames::EndingState::too_many_guess)),
    //                   std::to_string(game.global_stats().repeat_guess_count)});
  };

  auto vert = Container::Vertical(ai_buttons);
  // auto table = ftxui::Table(rows);
  // table.SelectAll().Border(BorderStyle::LIGHT);
  // auto fullview = Renderer(vert, [&]() {
  //   Elements internal{header, separator(), text("AI Overview")};
  //   for (auto &button : ai_buttons) {
  //     internal.push_back(button->Render());
  //   };
  //   internal.push_back(separator());
  //   // internal.push_back(table.Render());
  //
  //   return vbox(internal);
  // });
  //
  return vert;
}

void start(ProgramOptions::Options const &opt, std::vector<VirtualGames> &games)
/*std::vector<TestRunner> &results*/ {
  using namespace ftxui;
  // Start display things on the screen usign fxtui

  std::string input_text;
  // auto screen = ftxui::ScreenInteractive::Fullscreen();

  auto button_quit = Button("Quit", global::tabs.screen.ExitLoopClosure());

  auto header = Renderer(button_quit, [&] {
    return vbox(
        {hbox({text(std::format("Results from: {}", opt.program_to_test)) |
                   hcenter | vcenter | bold,
               button_quit->Render() | xflex}),
         separator()});
  });

  auto ButtonTest = Button("Yo", {});
  int tab_selected = 0;
  std::vector<std::string> tab_header_strings = {"ButtonTest", "Overview"};

  global::tabs.add_tab("Test", ButtonTest);
  global::tabs.add_tab("Overview", overview_tab(games));

  // screen.Loop(component);
}
// Use of this source code is governed by the MIT license that can be found
// in the LICENSE file.

} // namespace ui
