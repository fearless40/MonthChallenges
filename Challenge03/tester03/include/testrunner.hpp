#pragma once

#include "Array2D.hpp"
#include "aistats.hpp"
#include "programoptions.hpp"
#include "reproc++/reproc.hpp"
#include "reprochelper.hpp"
#include "ship.hpp"
#include <bitset>
#include <chrono>
#include <iostream>
#include <iterator>

class TestAI {
private:
  struct ShipHit {
    std::bitset<32> hits;

    constexpr bool is_sunk(battleship::ShipDefinition id) const {
      return hits.count() == id.size;
    }
  };

  using ShipHits = std::vector<ShipHit>;

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
  };

  ProgramOptions::Options const &m_opt;
  reproc::process m_app;
  battleship::GameLayout m_layout;
  std::vector<SingleRun> m_runs;

  AIStats_SingleRun total_stats;
  SingleRun m_current;

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
    std::array<std::string, 4> cmdline{m_opt.program_to_test, "run", "--ai",
                                       "0"};

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
    for (const auto &ship : m_current.ships) {
      std::cout << "Ship: " << ship.id().size << "RC: " << ship.location.y
                << ' ' << ship.location.x << '\n';
    }

    m_current.hits = make_hits_component(m_current.ships);

    m_current.start_time = std::chrono::steady_clock::now();
  }
  void end_test() {
    std::cout << "End Tests\n";
    m_runs.push_back(std::move(m_current));
  }

  void run_test() {
    std::cout << "Run Tests\n";
    unsigned char buffer[128];
    auto time = std::chrono::steady_clock::now();
    bool stillTesting = true;
    std::size_t byteRead{0};
    std::error_code ec;
    std::size_t count{0};
    while (stillTesting) {
      // auto event = m_app.poll(reproc::event::out, reproc::milliseconds(500));
      // if (event.first != reproc::event::out)
      // continue;
      std::tie(byteRead, ec) =
          m_app.read(reproc::stream::out, (unsigned char *)&buffer, 127);
      if (byteRead > 0 && !ec) {
        std::string_view recieved_text{(char *)buffer, byteRead};
        std::cout << "Read bytes: " << byteRead << " Read: " << recieved_text
                  << '\n';

        auto guess = battleship::RowCol::from_string(recieved_text);
        std::cout << "Guess: " << guess.as_colrow_fmt() << '\n';
        log_guess(guess, std::chrono::milliseconds(1));
        process_hit_logic(guess);
        if (std::chrono::steady_clock::now() - time >
            std::chrono::milliseconds{m_opt.wait_upto_millis}) {
          stillTesting = false;
        }
        if (++count > 100) {
          stillTesting = false;
          std::cout << "Loop too long count > 20.\n";
        }
      } else {
        std::cout << "Error: " << ec.message() << '\n';
        std::cout << "Stopping run\n";
        return;
      }
    }
  }

  void process_hit_logic(battleship::RowCol guess) {
    if (auto ship_opt = battleship::ship_at_position(m_current.ships, guess);
        ship_opt) {
      auto shipdef = ship_opt.value().id();

      auto index = m_layout.shipdef_to_index(shipdef);
      if (auto section_opt = m_current.ships[index].ship_section_hit(guess);
          section_opt) {
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
    std::cout << "Hit ship: " << shipdef.size << '\n';
    m_app.write((unsigned char *)"H\n", 2);
  }

  void miss_ship() {
    std::cout << "Miss\n";
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

  ShipHits static make_hits_component(battleship::Ships const &ships) {
    ShipHits hits{ships.size()};
    for (auto const &ship : ships) {
      hits.emplace_back(std::bitset<32>{});
    }
    return hits;
  }
};
