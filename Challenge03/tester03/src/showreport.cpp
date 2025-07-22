#include "showreport.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "programoptions.hpp"
#include "virtualgames.hpp"
#include <atomic>
#include <cstddef>
#include <exception>
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

ftxui::Component ToggleButton(std::string const &label, std::size_t tabID,
                              bool show_close = true);

namespace AppTabs {

struct TabMap {
  std::size_t id;
  ftxui::Component tab_button;
  ftxui::Component tab_content;
};

static std::size_t selected;
static int fake_selected;
static size_t nextID;

static ftxui::Component tabs_c;
static ftxui::Component container_c;
static std::vector<TabMap> tab_ID; // map of ID to position

static void set_active_tab(std::size_t tabID) {
  auto found = std::ranges::find(tab_ID, tabID, &TabMap::id);
  if (found != tab_ID.end()) {
    container_c->SetActiveChild(found->tab_content);
  }
}

static void close_tab(std::size_t tabID) {
  if (container_c->ChildCount() == 1)
    return;

  if (auto pairfind = std::ranges::find(tab_ID, tabID, &TabMap::id);
      pairfind != tab_ID.end()) {
    pairfind->tab_button->Detach();
    pairfind->tab_content->Detach();

    auto before = pairfind - 1;
    container_c->SetActiveChild(before->tab_content);
    tabs_c->SetActiveChild(before->tab_button);

    tab_ID.erase(pairfind);
  }
}

static void add_tab(std::string label, bool show_close, ftxui::Component comp) {
  auto tbutton = ToggleButton(label, nextID, show_close);

  if (!tabs_c) {
    tabs_c = ftxui::Container::Horizontal({tbutton});
    container_c = ftxui::Container::Tab({comp}, &fake_selected);
  } else {
    tabs_c->Add(tbutton);
    container_c->Add(comp);
    AppTabs::set_active_tab(tabs_c->ChildCount() - 1);
  }

  tab_ID.emplace_back(nextID, tbutton, comp);
  ++nextID;
}

static void init() {
  selected = 0;
  nextID = 0;
}
}; // namespace AppTabs
ftxui::Component ToggleButton(std::string const &label, std::size_t tabID,
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
  auto header =
      vbox({separator(),
            hbox({text(std::format("Total time: {}", 5)), separator(),
                  text(std::format("Total AIs Run: {}", games.size())),
                  separator(), text(std::format("Iterations: {}", 10))})});

  std::vector<std::string> ColHeaders = {"AI", "Average Guesses", "Won", "Lost",
                                         "Repeats"};
  std::vector<std::vector<std::string>> rows;
  Components ai_buttons;

  rows.push_back(std::move(ColHeaders));

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

    rows.push_back({std::to_string(game.aiid()),
                    std::to_string(game.global_stats().average_guess_count),
                    std::to_string(game.global_stats().ending_state(
                        VirtualGames::EndingState::sunk_all_ships)),
                    std::to_string(game.global_stats().ending_state(
                        VirtualGames::EndingState::too_many_guess)),
                    std::to_string(game.global_stats().repeat_guess_count)});
  };

  auto vert = Container::Vertical(ai_buttons);
  auto table = ftxui::Table(rows);
  table.SelectAll().Border(BorderStyle::LIGHT);
  table.SelectAll().Decorate(vcenter);
  table.SelectAll().Decorate(hcenter);
  table.SelectColumn(0).Decorate(bold);
  table.SelectRow(0).Decorate(bold);
  table.SelectAll().Border(LIGHT);
  table.SelectAll().Separator(LIGHT);

  auto tableElement = table.Render();

  return Renderer(vert, [header, vert, tableElement]() -> Element {
    return vbox({header, separator(), text("AI Overview") | bold,
                 vert->Render(), separator(), tableElement});
  });
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

  auto ButtonTest = Button("Yo", [&screen] {
    AppTabs::add_tab(
        "Garabage" + std::to_string(AppTabs::nextID + 1), true,
        ftxui::Button("Hello me" + std::to_string(AppTabs::nextID + 1),
                      std::bind(AiButtonClick, 1)));
  });

  AppTabs::add_tab("Overview", false, overview_tab(games));
  AppTabs::add_tab("Test", true, ButtonTest);
  AppTabs::add_tab("Weird", true,
                   ftxui::Button("Weird button", std::bind(AiButtonClick, 1)));

  screen.Loop(ftxui::Container::Vertical(
      {header, AppTabs::tabs_c, AppTabs::container_c}));
}
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

} // namespace ui
