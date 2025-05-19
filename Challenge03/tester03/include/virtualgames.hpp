#pragma once
#include "aistats.hpp"
#include "ship.hpp"
#include <bitset>
#include <chrono>

class VirtualGames {
  // Types used
public:
  using TimeT = std::chrono::microseconds;

  struct ShipHit {
    std::bitset<32> hits;

    constexpr bool is_sunk(battleship::ShipDefinition id) const {
      return hits.count() == id.size;
    }
  };

  using ShipHits = std::vector<ShipHit>;

  enum class GuessReport { Hit, Miss, Sink };

  struct GuessResult {
    GuessReport report;
    battleship::ShipDefinition ship;
  };

  enum class EndingState {
    timeout,
    program_error,
    other,
    too_many_guess,
    unable_read_output,
    sunk_all_ships,
    program_has_no_guesses,
    none
  };

  struct VirtualStats {
    TimeT shortest_answer{0};
    TimeT longest_answer{0};
    TimeT avg_answer{0};
    TimeT total_time{0};

    std::size_t repeat_guess_count{0};
    std::size_t invalid_guess_count{0};
    std::size_t average_guess_count{0};
  };
  ;

  struct Guess_Stats {
    battleship::RowCol guess;
    TimeT elapsed_time;
  };

  struct Game {
    std::chrono::steady_clock::time_point start_time;
    battleship::Ships ships;
    ShipHits hits;
    std::vector<Guess_Stats> guesses;
    VirtualStats stats;
    EndingState ending_state{EndingState::none};
  };

  // Public methods
public:
  VirtualGames() {};
  VirtualGames(std::string program, AIID ai, battleship::GameLayout layout)
      : program_name(program), id(ai), m_layout(layout) {};

  void new_game();
  void end_game();
  void start_guess_timer();
  void fail_game(EndingState state);
  void finish_games();
  GuessResult guess(const battleship::RowCol guess);
  void guess_program_has_no_more_guess();

  // Getters
  constexpr std::size_t get_current_guess_count() {
    return m_current.guesses.size();
  }

  // Private Methods
private:
  ShipHits static make_hits_component(battleship::Ships const &ships) {
    ShipHits hits{ships.size()};
    for (auto const &ship : ships) {
      hits.emplace_back(std::bitset<32>{});
    }
    return hits;
  }

  // Private Data
private:
  AIID id;
  std::string program_name{};
  battleship::GameLayout m_layout;

  Game m_current;
  std::vector<Game> m_games;
  std::chrono::high_resolution_clock::time_point m_guess_time;
};
