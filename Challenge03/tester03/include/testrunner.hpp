#pragma once

#include "Array2D.hpp"
#include "aistats.hpp"
#include "programoptions.hpp"
#include "reproc++/reproc.hpp"
#include "reprochelper.hpp"
#include "ship.hpp"
#include "virtualgames.hpp"
#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <system_error>

class TestAI {
  VirtualGames m_game;
  reproc::process m_app;
  std::string program;
  AIID m_id;

public:
  TestAI(ProgramOptions::Options const &options) : m_opt(options) {
    m_layout = {
        battleship::ShipDefinition{m_opt.smallestShip},
        battleship::ShipDefinition{m_opt.largestShip},
        battleship::Row{static_cast<battleship::Row::type>(m_opt.rowSize)},
        battleship::Col{static_cast<battleship::Col::type>(m_opt.colSize)}};
  }

  void start_tests(std::string program, AIID id, std::size_t nbrIterations,
                   battleship::GameLayout layout) {
    if (!initalize_app(program, id))
      return;
    m_game = VirtualGames(program, id, layout);
    for (std::size_t test_nbr = 0; test_nbr < nbrIterations; ++test_nbr) {
      begin_test();
      run_test();
    }
  };

private:
  bool initalize_app(std::string program, AIID id) {

    reproc::options options{default_process_options()};
    std::array<std::string, 4> cmdline{program, "run", "--ai",
                                       std::to_string(id)};

    auto ec = m_app.start(cmdline, options);

    if (ec == std::errc::no_such_file_or_directory) {
      std::cout << "No such program\n";
      return false;
    } else if (ec) {
      std::cout << "Error: " << ec.message() << '\n';
      return false;
    }
    return true;
  }

  void begin_test() { m_game.new_game(); }

  // void end_test() {
  //   m_game.end_game(VirtualGames::EndingState::sunk_all_ships);
  // }

  bool read_app() {
    unsigned char buffer[128];
    std::size_t byteRead{0};
    std::error_code ec;
    std::tie(byteRead, ec) =
        m_app.read(reproc::stream::out, (unsigned char *)&buffer, 127);

    if (byteRead > 0 && !ec) {
      std::string_view recieved_text{(char *)buffer, byteRead};

      auto guess = battleship::RowCol::from_string(recieved_text);
      auto result = m_game.guess(guess);
      switch (result.report) {
      case VirtualGames::GuessReport::Hit:
        hit_ship(result.ship);
        return true;
      case VirtualGames::GuessReport::Sink:
        sunk_ship(result.ship);
        return true;
      case VirtualGames::GuessReport::Miss:
        miss_ship();
        return true;
      }
    }
    return false;
  }

  void run_test() {
    std::cout << "Run Tests\n";
    std::size_t count{0};
    const std::size_t MAX_COUNT = m_game.max_guesses();

    while (1) {
      auto event = m_app.poll(reproc::event::out | reproc::event::deadline,
                              reproc::milliseconds(m_opt.wait_upto_millis));
      if (event.first == reproc::event::deadline) {
        m_game.end_game(VirtualGames::EndingState::timeout);
        return;
      } else if (event.second.value() != 0) {

        m_game.end_game(VirtualGames::EndingState::other);
        return;
      } else if (event.first == 0) {
        m_game.end_game(VirtualGames::EndingState::timeout);
        return;
      }
      if (!read_app()) {
        m_game.end_game(VirtualGames::EndingState::unable_read_output);
        return;
      }
      if (++count > MAX_COUNT) {
        m_game.end_game(VirtualGames::EndingState::too_many_guess);
        return;
      }
    }
  }

  void sunk_ship(battleship::ShipDefinition const shipdef) {
    std::cout << "Sunk ship: " << shipdef.size << '\n';
    std::string buf = std::format("S{}\n", shipdef.size);
    m_app.write((unsigned char *)buf.c_str(), buf.length());
  }

  void hit_ship(battleship::ShipDefinition const shipdef) {
    // std::cout << "Hit ship: " << shipdef.size << '\n';
    m_app.write((unsigned char *)"H\n", 2);
  }

  void miss_ship() {
    // std::cout << "Miss\n";
    m_app.write((unsigned char *)"M\n", 2);
  }

  //   void print_stats(SingleRun const &run) {
  //     auto &p = std::cout;
  //     p << "Stats for run: \n" << "-------------------------------------\n";
  //     p << "Total guesses: " << run.guesses.size() << '\n';
  //     p << "Shortest Guess: " << run.total_stats.shortest_answer << "\n";
  //     p << "Longest Guess: " << run.total_stats.longest_answer << "\n";
  //     p << "Repeat Guesses: " << run.total_stats.repeat_guess_count << '\n';
  //     p << "Invalid Guesses: " << run.total_stats.invalid_guess_count <<
  //     '\n';
  //   }
};
