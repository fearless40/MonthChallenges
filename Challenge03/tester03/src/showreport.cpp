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

ftxui::Component ToggleButton(std::string const &label, int tabID,
                              bool show_close = true);

namespace AppTabs {
static int selected;

static ftxui::Component tabs_c;
static ftxui::Component container_c;

static void add_tab(std::string label, bool show_close, ftxui::Component comp) {
  if (!tabs_c) {
    tabs_c = ftxui::Container::Horizontal({ToggleButton(label, 0, show_close)});
    container_c = ftxui::Container::Tab({comp}, &selected);
  } else {
    tabs_c->Add(ToggleButton(label, show_close, tabs_c->ChildCount() + 1));
    container_c->Add(comp);
  }
}

// static void remove_tab(std::string label) {
//   auto it = std::ranges::find(labels, label);
//   if (it != labels.end()) {
//     auto distance = std::distance(labels.begin(), it);
//     labels.erase(it);
//     components.erase(components.begin() + distance);
//     ftxui::ScreenInteractive::Active()->PostEvent(
//         ftxui::Event::Custom); // force redraw
//   }
// }

static void set_active_tab(int tabID) {
  selected = tabID;
  container_c->SetActiveChild(container_c->ChildAt(selected));
}

static void close_tab(int tabID) {
  if (container_c->ChildCount() == 1 || tabID >= container_c->ChildCount())
    return;

  auto childTab = container_c->ChildAt(tabID);
  childTab->Detach();

  container_c->SetActiveChild(container_c->ChildAt(tabID - 1));

  auto childLabel = tabs_c->ChildAt(tabID);
  childLabel->Detach();

  tabs_c->SetActiveChild(tabs_c->ChildAt(tabID - 1));

  // remove tab
}

static void init() { selected = 0; }
}; // namespace AppTabs
ftxui::Component ToggleButton(std::string const &label, int tabID,
                              bool show_close) {
  auto transform = [](const ftxui::EntryState &s) {
    auto element = ftxui::text(s.label);
    if (s.focused)
      element |= ftxui::bold;
    return element;
  };

  ftxui::ButtonOption label_btn_option;
  label_btn_option.label = label + (show_close ? "  " : "");
  label_btn_option.transform = transform;
  label_btn_option.on_click = std::bind(AppTabs::set_active_tab, tabID);

  ftxui::ButtonOption close_btn_option;
  close_btn_option.label = "X";
  close_btn_option.transform = transform;
  close_btn_option.on_click = std::bind(AppTabs::close_tab, tabID);

  if (show_close) {
    auto container = ftxui::Container::Horizontal(
        {ftxui::Button(label_btn_option), ftxui::Button(close_btn_option)});

    return ftxui::Renderer(container, [container] {
      return ftxui::hbox(container->Render(), ftxui::separator());
    });

  } else {
    auto container =
        ftxui::Container::Horizontal({ftxui::Button(label_btn_option)});

    return ftxui::Renderer(container, [container] {
      return ftxui::hbox(container->Render(), ftxui::separator());
    });
  }
}

void AiButtonClick(AIID id) {}
// std::accumulate(games.begin(), games.end(),
//                            std::chrono::milliseconds(0),
//                            [](std::chrono::milliseconds const &time,
//                               VirtualGames const &value) {
//                              return time +
//                              value.global_stats().total_time;
//
ftxui::Component overview_tab(std::vector<VirtualGames> const &games) {
  using namespace ftxui;
  // auto header = hbox({text(std::format("Total time: {}", 5)),
  // separator(),
  //                     text(std::format("Total AIs Run: {}",
  //                     games.size()))});
  //
  // std::vector<std::string> ColHeaders = {"AI", "Average Guesses",
  // "Won", "Lost",
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
  auto screen = ftxui::ScreenInteractive::Fullscreen();
  std::string input_text;
  AppTabs::init();

  auto button_quit = Button("Quit", screen.ExitLoopClosure());

  auto header = Renderer(button_quit, [&] {
    return vbox(
        {hbox({text(std::format("Results from: {}", opt.program_to_test)) |
                   hcenter | vcenter | bold,
               button_quit->Render() | xflex}),
         separator()});
  });

  auto ButtonTest = Button("Yo", {});

  AppTabs::add_tab("Overview", false, overview_tab(games));
  AppTabs::add_tab("Test", true, ButtonTest);

  screen.Loop(ftxui::Container::Vertical(
      {header, AppTabs::tabs_c, AppTabs::container_c}));
}
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

} // namespace ui
