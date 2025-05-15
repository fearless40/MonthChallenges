#pragma once

#include "Array2D.hpp"
#include "aistats.hpp"
#include "programoptions.hpp"
#include "reproc++/reproc.hpp"
#include "reprochelper.hpp"
#include "ship.hpp"
#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <system_error>

class TestAI {
private:
  struct ShipHit {
    std::bitset<32> hits;

    constexpr bool is_sunk(battleship::ShipDefinition id) const {
      return hits.count() == id.size;
    }
  };

  using ShipHits = std::vector<ShipHit>;

  enum class FailReasons {
    timeout,
    program_error,
    other,
    too_many_guess,
    unable_read_output,
    none
  };

  struct Guess_Stats {
    battleship::RowCol guess;
    std::chrono::milliseconds elapsed_time;
  };

  struct SingleRun {
    std::chrono::steady_clock::time_point start_time;
    battleship::Ships ships;
    ShipHits hits;
    // battleship::Array2D<std::size_t> guess_repeats;
    std::vector<Guess_Stats> guesses;
    AIStats_SingleRun total_stats;
    FailReasons failed_run{FailReasons::none};
  };

  ProgramOptions::Options const &m_opt;
  reproc::process m_app;
  battleship::GameLayout m_layout;
  std::vector<SingleRun> m_runs;

  AIStats_SingleRun total_stats;
  SingleRun m_current;
  std::chrono::steady_clock::time_point m_current_guess_start;

public:
  TestAI(const TestAI &) = delete;
  TestAI(TestAI &&) = default;
  TestAI &operator=(const TestAI &) = delete;
  TestAI &operator=(TestAI &&) = delete;
  TestAI(ProgramOptions::Options const &options) : m_opt(options) {
    m_layout = {
        battleship::ShipDefinition{m_opt.smallestShip},
        battleship::ShipDefinition{m_opt.largestShip},
        battleship::Row{static_cast<battleship::Row::type>(m_opt.rowSize)},
        battleship::Col{static_cast<battleship::Col::type>(m_opt.colSize)}};
  }

  void initalize_app() {

    reproc::options options{default_process_options()};
    std::array<std::string, 4> cmdline{
        m_opt.program_to_test, "run", "--ai",
        std::to_string(m_opt.ai_id_to_test.front())};

    auto ec = m_app.start(cmdline, options);

    if (ec == std::errc::no_such_file_or_directory) {
      std::cout << "No such program\n";
    } else if (ec) {
      std::cout << "Error: " << ec.message() << '\n';
    }
  }

  void start_tests() {
    for (std::size_t test_nbr = 0; test_nbr < m_opt.nbrIterations; ++test_nbr) {
      begin_test();
      run_test();
      end_test();
    }
  };

private:
  ShipHits static make_hits_component(battleship::Ships const &ships) {
    ShipHits hits{ships.size()};
    for (auto const &ship : ships) {
      hits.emplace_back(std::bitset<32>{});
    }
    return hits;
  }

  void begin_test() {
    // Initlize m_current
    std::cout << "Begin Tests\n";
    m_current.guesses.reserve(m_layout.nbrCols.size * m_layout.nbrRows.size +
                              10);
    if (auto opt = battleship::random_ships(m_layout); opt) {
      m_current.ships = opt.value();
    } else {
      std::cout << "Could not generate ships\n";
    }

    m_current.hits = make_hits_component(m_current.ships);

    m_current.start_time = std::chrono::steady_clock::now();
  }

  void end_test() {
    std::cout << "End Tests\n";

    auto result = std::ranges::minmax_element(m_current.guesses, {},
                                              &Guess_Stats::elapsed_time);
    m_current.total_stats.shortest_answer = result.min->elapsed_time;
    m_current.total_stats.longest_answer = result.max->elapsed_time;

    m_current.total_stats.avg_answer =
        std::accumulate(
            m_current.guesses.begin() + 1, m_current.guesses.end(),
            m_current.guesses.begin()->elapsed_time,
            [](auto const &a, auto const &b) { return a + b.elapsed_time; }) /
        m_current.guesses.size();

    // m_current.total_stats.total_time

    print_stats(m_current);
    m_runs.push_back(std::move(m_current));
  }

  bool run_test_read_output() {
    unsigned char buffer[128];
    std::size_t byteRead{0};
    std::error_code ec;
    std::tie(byteRead, ec) =
        m_app.read(reproc::stream::out, (unsigned char *)&buffer, 127);

    if (byteRead > 0 && !ec) {
      std::string_view recieved_text{(char *)buffer, byteRead};
      // std::cout << "Read bytes: " << byteRead << " Read: " << recieved_text
      // << '\n';

      auto guess = battleship::RowCol::from_string(recieved_text);
      // std::cout << "Guess: " << guess.as_colrow_fmt() << '\n';
      log_guess(guess,
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - m_current_guess_start));
      process_hit_logic(guess);
      m_current_guess_start = std::chrono::steady_clock::now();

      return true;
    }
    return false;
  }

  void run_test() {
    std::cout << "Run Tests\n";
    std::size_t count{0};
    const std::size_t MAX_COUNT =
        (m_layout.nbrCols.size * m_layout.nbrRows.size) + 10;

    while (1) {
      auto event = m_app.poll(reproc::event::out | reproc::event::deadline,
                              reproc::milliseconds(m_opt.wait_upto_millis));
      if (event.first == reproc::event::deadline) {
        end_run(FailReasons::timeout);
        return;
      } else if (event.second.value() != 0) {
        end_run(FailReasons::other);
        return;
      } else if (event.first == 0) {
        end_run(FailReasons::timeout);
        return;
      }
      if (!run_test_read_output()) {
        end_run(FailReasons::unable_read_output);
        return;
      }
      if (++count > MAX_COUNT) {
        end_run(FailReasons::too_many_guess);
        std::cout << "Loop too long count > " << MAX_COUNT << '\n';
        return;
      }
    }
  }

  void end_run(FailReasons reason) {
    std::cout << "Run failed...\n";
    m_current.failed_run = reason;
  }

  void process_hit_logic(battleship::RowCol guess) {
    if (auto ship_opt = battleship::ship_at_position(m_current.ships, guess);
        ship_opt) {
      auto shipdef = ship_opt.value().id();

      auto index = m_layout.shipdef_to_index(shipdef);
      if (auto section_opt = m_current.ships[index].ship_section_hit(guess);
          section_opt) {
        std::cout << "Hit ship: " << shipdef.size
                  << " at index: " << section_opt.value() << '\n';
        m_current.hits[index].hits.set(section_opt.value(), true);
        if (m_current.hits[index].is_sunk(shipdef)) {
          sunk_ship(shipdef);
        } else {
          hit_ship(shipdef);
        }
      }
    } else
      miss_ship();
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

  void log_guess(battleship::RowCol guess,
                 std::chrono::milliseconds elasped_time) {
    auto index =
        std::ranges::find(m_current.guesses, guess, &Guess_Stats::guess);
    if (index != m_current.guesses.end()) {
      ++m_current.total_stats.repeat_guess_count;
    }
    m_current.guesses.emplace_back(guess, elasped_time);
    m_current.total_stats.shortest_answer =
        std::min(m_current.total_stats.shortest_answer, elasped_time);
    m_current.total_stats.longest_answer =
        std::max(m_current.total_stats.longest_answer, elasped_time);
  }

  void print_stats(SingleRun const &run) {
    auto &p = std::cout;
    p << "Stats for run: \n" << "-------------------------------------\n";
    p << "Total guesses: " << run.guesses.size() << '\n';
    p << "Shortest Guess: " << run.total_stats.shortest_answer << "\n";
    p << "Longest Guess: " << run.total_stats.longest_answer << "\n";
    p << "Repeat Guesses: " << run.total_stats.repeat_guess_count << '\n';
    p << "Invalid Guesses: " << run.total_stats.invalid_guess_count << '\n';
  }
};
