#include "showreport.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "programoptions.hpp"
#include "virtualgames.hpp"
#include <chrono>
#include <cstddef>
#include <format>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/dom/table.hpp>
#include <functional>

#include <fstream>
#include <ios>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
namespace ui {

std::fstream log("data.txt", std::ios_base::out);

namespace AppTabs {

ftxui::Component TabButton(std::string const &label, std::size_t tabID,
                           bool show_close = true);
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
  auto tbutton = AppTabs::TabButton(label, nextID, show_close);

  tabs_c->Add(tbutton);
  container_c->Add(comp);
  tab_ID.emplace_back(nextID, tbutton, comp);
  AppTabs::set_active_tab(nextID);

  ++nextID;
}

static void init() {
  selected = 0;
  tabs_c = ftxui::Container::Horizontal({});
  container_c = ftxui::Container::Tab({}, &fake_selected);
  nextID = 0;
}

ftxui::Component TabButton(std::string const &label, std::size_t tabID,
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

}; // namespace AppTabs

void AiButtonClick(AIID id) {}
// std::accumulate(games.begin(), games.end(),
//                            std::chrono::milliseconds(0),
//                            [](std::chrono::milliseconds const &time,
//                               VirtualGames const &value) {
//                              return time +
//                              value.global_stats().total_time;
//
ftxui::Component overview_tab(std::vector<VirtualGames> const &games,
                              ProgramOptions::Options const &options) {
  using namespace ftxui;

  auto total_time = std::accumulate(
      games.begin(), games.end(), std::chrono::milliseconds{0},
      [](std::chrono::milliseconds elapsed, const VirtualGames &game) {
        return elapsed + std::chrono::duration_cast<std::chrono::milliseconds>(
                             game.global_stats().total_time);
      });

  // clang-format off
  auto header = vbox({
      separator(),
      hbox({
         text(std::format("Total time: {}", std::chrono::hh_mm_ss(total_time))),
         separator(),
         text(std::format("Total AIs Run: {}", games.size())),
         separator(),
         text(std::format("Iterations: {}", options.nbrIterations))
      })
   });

  // clang-format on
  std::vector<std::vector<std::string>> data_table_rows;

  data_table_rows.push_back(
      {"AI", "Average Guesses", "Won", "Lost", "Repeats"});

  std::vector<std::vector<Element>> ai_table_elements;

  ai_table_elements.push_back({text("AI ID (click for details)"),
                               text("Average Guess per game"),
                               text("Graph based on lowest guess count")});

  auto ai_button_vert_container = Container::Vertical({});
  auto min_guesses =
      std::ranges::min_element(games, std::less<>{},
                               [](VirtualGames const &game) {
                                 return game.global_stats().average_guess_count;
                               })
          ->global_stats()
          .average_guess_count;

  for (auto const &game : games) {

    auto ai_button =
        Button(std::format("AI {}", game.aiid()),
               std::bind(AiButtonClick, game.aiid()),
               ButtonOption::Animated(Color::Palette256::BlueViolet));
    ai_button_vert_container->Add(ai_button);

    ai_table_elements.push_back(
        {ai_button->Render(),
         text(std::to_string(game.global_stats().average_guess_count)),
         gaugeRight(static_cast<float>(game.global_stats().average_guess_count -
                                       min_guesses) /
                    static_cast<float>(min_guesses))});

    data_table_rows.push_back(
        {std::to_string(game.aiid()),
         std::to_string(game.global_stats().average_guess_count),
         std::to_string(game.global_stats().ending_state(
             VirtualGames::EndingState::sunk_all_ships)),
         std::to_string(game.global_stats().ending_state(
             VirtualGames::EndingState::too_many_guess)),
         std::to_string(game.global_stats().repeat_guess_count)});
  };

  auto table = ftxui::Table(data_table_rows);
  table.SelectAll().Border(BorderStyle::LIGHT);
  table.SelectAll().Decorate(vcenter);
  table.SelectAll().Decorate(hcenter);
  table.SelectColumn(0).Decorate(bold);
  table.SelectRow(0).Decorate(bold);
  table.SelectAll().Border(LIGHT);
  table.SelectAll().Separator(LIGHT);

  return Renderer(ai_button_vert_container, [&] {
    auto table_interactive = Table(ai_table_elements);

    table_interactive.SelectAll().Decorate(vcenter);

    table_interactive.SelectAll().Border(ftxui::LIGHT);
    table_interactive.SelectAll().Separator(ftxui::LIGHT);

    return vbox({header, separator(), text("AI Avg Guess Overview") | bold,
                 table_interactive.Render(), separator(), text("Stat Overview"),
                 table.Render()});
  });
}

ftxui::Component ai_tab(const VirtualGames &games) {
  using namespace ftxui;

  std::size_t *current_selected_game = new std::size_t(
      0); // Memory leak on purpose. Will be cleaned up when the program ends.

  struct SelectableCell : ComponentBase {
    SelectableCell(std::size_t id, const std::string text,
                   std::size_t *current_selection)
        : game_id(id), cell_text(text), current_selected_id(current_selection) {
    }

    std::size_t game_id;
    std::string cell_text;
    std::size_t *current_selected_id;
    Box box;

    Element OnRender() final {
      Element element;

      if (*current_selected_id == game_id)
        element = text(cell_text) | ftxui::bgcolor(Color(40, 200, 80)) | focus;
      else
        element = text(cell_text);

      element |= reflect(box);
      return element;
    }

    bool Focusable() const final { return true; }

    bool OnEvent(Event event) final {
      bool mouse_hover =
          box.Contain(event.mouse().x, event.mouse().y) && CaptureMouse(event);

      if ((event.mouse().button == Mouse::Left &&
           event.mouse().motion == Mouse::Pressed && mouse_hover) ||
          event == Event::Return) {
        *current_selected_id = game_id;
        TakeFocus();
        return true;
      }
      return false;
    }
  };

  int *selected_x = new int(0);
  auto game_rows = Container::Vertical({}, selected_x);

  int *selected_y = new int(0);
  std::vector<Elements> table_elements;
  table_elements.push_back(
      Elements{text("Round"), text("Status"), text("Guess #")});

  std::size_t count{0};
  for (auto &game : games.all_games()) {
    ++count;
    // log << count << '\n';
    auto cellid = Make<SelectableCell>(count, std::to_string(count),
                                       current_selected_game);
    auto cellstatus = Make<SelectableCell>(
        count, VirtualGames::EndingState_ToString(game.ending_state),
        current_selected_game);
    auto cellguess = Make<SelectableCell>(
        count, std::to_string(game.guesses.size()), current_selected_game);

    auto row =
        Container::Horizontal({cellid, cellstatus, cellguess}, selected_y);
    game_rows->Add(row);

    Elements tb_row{cellid->Render(), cellstatus->Render(),
                    cellguess->Render()};
    table_elements.push_back(std::move(tb_row));
  }

  auto left_table = Renderer(game_rows, [tbe = std::move(table_elements)] {
    auto table = Table(tbe);
    table.SelectAll().Border(ftxui::LIGHT);
    table.SelectAll().Separator(ftxui::LIGHT);

    return table.Render() | vscroll_indicator | focusPosition(0, 0) | frame;
  });

  auto right_details = Button("Details", std::bind(AiButtonClick, 1));
  int *left_size = new int(30);
  return ResizableSplitLeft(left_table, right_details, left_size);
};

void start(ProgramOptions::Options const &opt, std::vector<VirtualGames> &games)
/*std::vector<TestRunner> &results*/ {
  using namespace ftxui;
  // Start display things on the screen usign fxtui
  auto screen = ftxui::ScreenInteractive::Fullscreen();
  std::string input_text;
  AppTabs::init();

  auto button_quit = Button("Quit", screen.ExitLoopClosure());

  auto header_container = Container::Horizontal({button_quit});

  // clang-format off
  auto header = Renderer(header_container, [&] {
    return vbox( {
         hbox({
            text(std::format("Results from: {}", opt.program_to_test)) |
            hcenter | vcenter | bold, button_quit->Render() |
            size(ftxui::WIDTH, Constraint::EQUAL, 8)
         }),
         separator()
         }) ;
  });
  // clang-format on

  AppTabs::add_tab("Overview", false, overview_tab(games, opt));

  std::size_t count = 0;
  for (auto &game : games) {
    AppTabs::add_tab(std::format("Run {}, AI ID {}", ++count, game.aiid()),
                     true, ai_tab(game));
  }

  screen.Loop(ftxui::Container::Vertical(
      {header, AppTabs::tabs_c, AppTabs::container_c}));
}

} // namespace ui
