#include "showreport.hpp"
#include "RowCol.hpp"
#include "ftxui/component/component_options.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "programoptions.hpp"
#include "ship.hpp"
#include "virtualgames.hpp"
#include "widgets.hpp"
#include <chrono>
#include <cstddef>
#include <format>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/linear_gradient.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/string.hpp>

#include <algorithm>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

namespace ui {

std::fstream log("data.txt", std::ios_base::out);

std::shared_ptr<Widgets::DynamicTab> Tabs;

void OpenAITab(const VirtualGames &game);

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

  auto ai_button_vert_container = Container::Vertical({});

  for (auto const &game : games) {

    auto ai_button =
        Button(std::format("AI {}", game.aiid()), std::bind(OpenAITab, game),
               ButtonOption::Animated(Color::Palette256::BlueViolet));
    ai_button_vert_container->Add(ai_button);
  }

  auto total_time = std::accumulate(
      games.begin(), games.end(), std::chrono::milliseconds{0},
      [](std::chrono::milliseconds elapsed, const VirtualGames &game) {
        return elapsed + std::chrono::duration_cast<std::chrono::milliseconds>(
                             game.global_stats().total_time);
      });
  auto min_guesses =
      std::ranges::min_element(games, std::less<>{},
                               [](VirtualGames const &game) {
                                 return game.global_stats().average_guess_count;
                               })
          ->global_stats()
          .average_guess_count;

  return Renderer(ai_button_vert_container, [ai_button_vert_container,
                                             min_guesses, total_time, &games,
                                             &options] {
    // clang-format off
     auto header = vbox({
         hbox({
            text(std::format("Total time: {}", std::chrono::hh_mm_ss(total_time))),
            separator(),
            text(std::format("Total AIs Run: {}", games.size())),
            separator(),
            text(std::format("Iterations: {}", options.nbrIterations))
         })
      });

    // clang-format on
    std::vector<std::vector<Element>> ai_table_elements;
    std::vector<std::vector<std::string>> data_table_rows;
    data_table_rows.push_back(
        {"AI", "Average Guesses", "Won", "Lost", "Repeats"});

    ai_table_elements.push_back({text("AI ID (click for details)"),
                                 text("Average Guess per game"),
                                 text("Graph based on lowest guess count")});
    std::size_t count = 0;
    for (auto &game : games) {
      // clang-format off
         ai_table_elements.push_back({ 
            ai_button_vert_container->ChildAt(count)->Render(),
            text(std::to_string(game.global_stats().average_guess_count)),
            gaugeRight(static_cast<float>(game.global_stats().average_guess_count -
                        min_guesses) / static_cast<float>(min_guesses))
         });
      // clang-format on
      ++count;

      data_table_rows.push_back(
          {std::to_string(game.aiid()),
           std::to_string(game.global_stats().average_guess_count),
           std::to_string(game.global_stats().ending_state(
               VirtualGames::EndingState::sunk_all_ships)),
           std::to_string(game.global_stats().ending_state(
               VirtualGames::EndingState::too_many_guess)),
           std::to_string(game.global_stats().repeat_guess_count)});
    }

    auto table_ai = Table(ai_table_elements);
    table_ai.SelectAll().Border(LIGHT);
    table_ai.SelectAll().Separator(LIGHT);

    auto data_ai = Table(data_table_rows);
    data_ai.SelectAll().Border(LIGHT);
    data_ai.SelectAll().Separator(LIGHT);

    return vbox({header, table_ai.Render(), separator(), data_ai.Render()});

    //  return vbox({header, separator(), text("AI Avg Guess Overview") | bold,
    //                table_interactive.Render(), separator(),
    //                text("Stat Overview"), table.Render()});
  });
}

// namespace Widgets

ftxui::Color get_color_on_status(VirtualGames::Guess_Stats_Result result) {
  using Color = ftxui::Color;
  switch (result) {
    using enum VirtualGames::Guess_Stats_Result;
  case hit:
    return Color(0, 255, 0);
  case miss:
    return Color(255, 0, 0);
  case invalid:
    return Color(255, 255, 0);
  case sunk:
    return Color(0, 0, 255);
  case unknown:
    return Color(100, 200, 90);
  default:
    return Color(255, 255, 255);
  }
};

ftxui::Component game_tab(const VirtualGames &definition,
                          const VirtualGames::Game &game) {

  using namespace ftxui;

  // Shows the moves on the left using interactive table
  // Show the game board on the right showing ship positions.
  //  When clicking on a move show on the right table the location of the move
  //  Be able to play back moves
  //  Filter moves
  // Use different colors to indicate hit/miss/repeat

  std::vector<std::string> headers = {"Guess Nbr", "Position", "Status",
                                      "Time"};
  std::vector<std::string> values;

  values.reserve(game.guesses.size() * headers.size());

  std::size_t count = 1;
  for (auto const &round : game.guesses) {
    values.push_back(std::format("{}", count));
    values.push_back(round.guess.as_base26_fmt());
    values.push_back(round.result_as_string());
    values.push_back(std::format("{}", round.elapsed_time));
    ++count;
  }

  auto right_side =
      std::make_shared<Widgets::SelectableTable>(std::move(headers), values);

  auto left_side = std::make_shared<Widgets::GameBoard>();

  auto splitter =
      ResizableSplitLeft(right_side, left_side, &right_side->left_size);

  return Renderer(splitter, [splitter, right_side, left_side, &game,
                             &definition]() {
    std::vector<Widgets::GameBoard::DisplayPoint> points;
    auto active_point = game.guesses[right_side->get_selected_row()];
    for (std::size_t row = 0; row < definition.layout().nbrRows.size; ++row) {
      for (std::size_t col = 0; col < definition.layout().nbrCols.size; ++col) {
        auto status = battleship::shot_at(
            game.ships, battleship::RowCol{
                            battleship::Row{static_cast<unsigned short>(row)},
                            battleship::Col{static_cast<unsigned short>(col)}});

        if (active_point.guess.row.size == row &&
            active_point.guess.col.size == col) {
          points.emplace_back(get_color_on_status(active_point.result),
                              Color(0, 0, 0), "X");
        } else if (status.id.size != 0) {
          points.emplace_back(Color(200, 200, 80), Color(0, 0, 0, 0),
                              std::to_string(status.id.size));
        } else {
          points.emplace_back(Color(0, 0, 0), Color(0, 0, 0, 0), " ");
        }
      }
    }

    left_side->set_board(std::move(points), definition.layout().nbrCols.size);

    return splitter->Render();
  });
};

ftxui::Component ai_tab(const VirtualGames &games) {
  using namespace ftxui;

  std::vector<std::string> data;
  std::vector<std::string> headers{"Round", "Status", "Guess Count"};
  std::size_t count{0};

  for (auto &game : games.all_games()) {
    ++count;
    data.push_back(std::to_string(count));
    data.push_back(VirtualGames::EndingState_ToString(game.ending_state));
    data.push_back(std::to_string(game.guesses.size()));
  }

  auto left_side =
      std::make_shared<Widgets::SelectableTable>(std::move(headers), data);

  auto show_active_game_tab = [left_side, &games]() {
    Tabs->add_tab(
        std::format("AI: {}, Game {}", games.aiid(),
                    left_side->get_selected_row() + 1),
        true,
        game_tab(games, games.all_games()[left_side->get_selected_row()]));

    // Make_tab(left_side->get_selected_game)
  };

  auto right_button = Button("Show Game details", show_active_game_tab);

  auto right_details = Renderer(right_button, [right_button, left_side,
                                               &games]() {
    std::vector<std::vector<std::string>> table_data;

    auto game = games.all_games()[left_side->get_selected_row()];

    table_data.push_back({"Ending State:", VirtualGames::EndingState_ToString(
                                               game.ending_state)});
    table_data.push_back(
        {"Total guesses:", std::to_string(game.guesses.size())});
    table_data.push_back(
        {"Total Time:", std::format("{}", game.stats.total_time)});
    table_data.push_back({"Invalid Guess count:",
                          std::format("{}", game.stats.invalid_guess_count)});
    table_data.push_back({"Repeat Guess count:",
                          std::format("{}", game.stats.repeat_guess_count)});
    table_data.push_back(
        {"Longest answer time:", std::format("{}", game.stats.longest_answer)});
    table_data.push_back({"Shortest answer time:",
                          std::format("{}", game.stats.shortest_answer)});
    table_data.push_back(
        {"Average answer time", std::format("{}", game.stats.avg_answer)});

    auto table = Table(table_data);
    table.SelectAll().Border(LIGHT);
    table.SelectAll().Separator(LIGHT);

    return vbox({right_button->Render(), table.Render()});
  });

  return ResizableSplitLeft(left_side, right_details, &left_side->left_size);
};

void OpenAITab(const VirtualGames &game) {
  Tabs->add_tab(std::format("AI ID {}", game.aiid()), true, ai_tab(game));
}

void start(ProgramOptions::Options const &opt, std::vector<VirtualGames> &games)
/*std::vector<TestRunner> &results*/ {
  using namespace ftxui;
  // Start display things on the screen usign fxtui
  auto screen = ftxui::ScreenInteractive::Fullscreen();
  std::string input_text;

  auto button_quit = Button("Quit", screen.ExitLoopClosure());

  auto header_container = Container::Horizontal({button_quit});

  // clang-format off
  auto header = Renderer(header_container, [&] {
    return 
         hbox({
            text(std::format("Results from: {}", opt.program_to_test)) |
            hcenter | vcenter | bold, button_quit->Render() |
            size(ftxui::WIDTH, Constraint::EQUAL, 8)
         }) | size(ftxui::HEIGHT, Constraint::EQUAL, 3);
  });
  // clang-format on

  Tabs = std::make_shared<Widgets::DynamicTab>();

  Tabs->add_tab("Overview", false, overview_tab(games, opt));

  // std::size_t count = 0;
  // for (auto &game : games) {
  //   Tabs->add_tab(std::format("Run {}, AI ID {}", ++count,
  //   game.aiid()), true,
  //                 ai_tab(game));
  // }

  screen.Loop(ftxui::Container::Vertical({header, Tabs}));
}

} // namespace ui
