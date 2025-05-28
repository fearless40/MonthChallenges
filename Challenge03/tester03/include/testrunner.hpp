#pragma once

#include "aistats.hpp"
#include "programoptions.hpp"
#include "reproc++/reproc.hpp"
#include "reprochelper.hpp"
#include "virtualgames.hpp"
#include <cstddef>
#include <iostream>
#include <system_error>

class TestRunner {
  VirtualGames m_game;
  reproc::process m_app;
  reproc::milliseconds m_timeout;

public:
  TestRunner(ProgramOptions::Options const &options, AIID aiid)
      : m_timeout(options.wait_upto_millis) {
    m_game = VirtualGames(
        options.program_to_test, aiid,
        {battleship::ShipDefinition{options.smallestShip},
         battleship::ShipDefinition{options.largestShip},
         battleship::Row{static_cast<battleship::Row::type>(options.rowSize)},
         battleship::Col{static_cast<battleship::Col::type>(options.colSize)}});
  }

  void start_tests(std::size_t nbrIterations) {
    if (!initalize_app(m_game.program_name(), m_game.aiid()))
      return;
    for (std::size_t test_nbr = 0; test_nbr < nbrIterations; ++test_nbr) {
      begin_test();
      run_test();
    }

    send_quit();
  };

  const VirtualGames &games() const { return m_game; }

private:
  bool initalize_app(std::string program, AIID id) {

    reproc::options options{default_process_options()};
    std::array<std::string, 4> cmdline{program, "run", "--ai",
                                       std::to_string(id)};

    auto ec = m_app.start(cmdline, options);

    if (ec == std::errc::no_such_file_or_directory) {
      // std::cout << "No such program\n";
      return false;
    } else if (ec) {
      // std::cout << "Error: " << ec.message() << '\n';
      return false;
    }
    return true;
  }

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

      if (buffer[0] == '-') {
        m_game.end_game(VirtualGames::EndingState::program_has_no_guesses);
        return true;
      }

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
      auto event =
          m_app.poll(reproc::event::out | reproc::event::deadline, m_timeout);
      if (event.first == reproc::event::deadline) {
        m_game.end_game(VirtualGames::EndingState::timeout);
        std::cout << "Timeout\n";
        return;
      } else if (event.second.value() != 0) {

        std::cout << "other\n";
        m_game.end_game(VirtualGames::EndingState::other);
        return;
      } else if (event.first == 0) {
        std::cout << "Timeout\n";
        m_game.end_game(VirtualGames::EndingState::timeout);
        return;
      }
      if (!read_app()) {
        std::cout << "cannot read output\n";
        m_game.end_game(VirtualGames::EndingState::unable_read_output);
        return;
      }
      if (++count > MAX_COUNT) {
        std::cout << "max count\n";
        m_game.end_game(VirtualGames::EndingState::too_many_guess);
        return;
      }
      if (m_game.sunk_all_ships()) {
        std::cout << "Sunk all shipts\n";
        m_game.end_game(VirtualGames::EndingState::sunk_all_ships);
        return;
      }
    }
  }
  void begin_test() {
    m_app.write((unsigned char *)"E\n", 2);
    m_game.new_game();
  }

  void sunk_ship(battleship::ShipDefinition const shipdef) {
    std::cout << "Sunk ship: " << shipdef.size << '\n';
    std::string buf = std::format("S{}\n", shipdef.size);
    m_app.write((unsigned char *)buf.c_str(), buf.length());
    m_game.start_guess_timer();
  }

  void hit_ship(battleship::ShipDefinition const shipdef) {
    std::cout << "Hit ship: " << shipdef.size << '\n';
    m_app.write((unsigned char *)"H\n", 2);
    m_game.start_guess_timer();
  }

  void miss_ship() {
    std::cout << "Miss\n";
    m_app.write((unsigned char *)"M\n", 2);
    m_game.start_guess_timer();
  }
  void send_quit() {
    std::cout << "Miss\n";
    m_app.write((unsigned char *)"Q\n", 2);
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
