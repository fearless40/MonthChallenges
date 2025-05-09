#pragma once

#include "Array2D.hpp"
#include "aistats.hpp"
#include "programoptions.hpp"
#include "reproc++/reproc.hpp"
#include "reprochelper.hpp"
#include "ship.hpp"
#include <bitset>
#include <chrono>

class TestAI {
private:
  struct ShipHit {
    std::bitset<32> hits;

    constexpr bool is_sunk(battleship::ShipDefinition id) const {
      return hits.count() == id.size;
    }
  };

  using ShipHits = std::vector<ShipHit>;

  struct SingleRun {
    std::chrono::steady_clock::time_point start_time;
    std::size_t guess_nbr{0};
    battleship::Ships ships;
    ShipHits hits;
    battleship::Array2D<std::size_t> guess_repeats;
  };

  ProgramOptions::Options const &m_opt;
  reproc::process m_app;
  battleship::GameLayout m_layout;
  std::vector<SingleRun> m_runs;

  AIStats_SingleRun total_stats;

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
    if (auto ship_op = battleship::random_ships(m_layout); ship_op) {
      m_ships = ship_op.value();
      m_hits = TestAI::make_hits_component(m_ships);
    }
  }

  void initalize_app() {

    reproc::process app;
    reproc::options options{default_process_options()};
    std::array<std::string, 4> cmdline{m_opt.program_to_test, "run", "--ai",
                                       "0"};

    auto ec = app.start(cmdline, options);

    if (ec == std::errc::no_such_file_or_directory) {
      return;
    }
  }

  void start_tests() {
    for (std::size_t test_nbr = 0; test_nbr < m_opt.nbrIterations; ++test_nbr) {
      begin_test();
      run_test();
      end_test();
    }
  };

  void begin_test() {}
  void end_test() {}

  void run_test() {
    unsigned char buffer[128];
    auto time = std::chrono::steady_clock::now();
    bool stillTesting = true;
    std::size_t byteRead{0};
    std::error_code ec;
    while (stillTesting) {
      std::tie(byteRead, ec) =
          m_app.read(reproc::stream::out, (unsigned char *)&buffer, 127);
      if (byteRead > 0) {
        auto guess = battleship::RowCol::from_string(
            std::string_view{(char *)&buffer, byteRead});
        log_guess(guess);
        process_hit_logic(guess);
        if (std::chrono::steady_clock::now() - time >
            std::chrono::milliseconds{m_opt.wait_upto_millis}) {
          stillTesting = false;
        }
      }
    }
  }

  void process_hit_logic(battleship::RowCol guess) {
    if (auto ship_opt = battleship::ship_at_position(m_ships, guess);
        ship_opt) {
      auto shipdef = ship_opt.value().id();

      auto index = m_layout.shipdef_to_index(shipdef);
      if (auto section_opt = m_ships[index].ship_section_hit(guess);
          section_opt) {
        m_hits[index].hits.set(section_opt.value(), true);
        if (m_hits[index].is_sunk(shipdef)) {
          sunk_ship(m_app, shipdef);
        } else {
          hit_ship(m_app, shipdef);
        }
      }
    }
  }

private:
  void log_guess(battleship::RowCol guess,
                 std::chrono::milliseconds elasped_time) {}

  ShipHits static make_hits_component(battleship::Ships const &ships) {
    ShipHits hits{ships.size()};
    for (auto const &ship : ships) {
      hits.emplace_back(std::bitset<32>{});
    }
    return hits;
  }
};
