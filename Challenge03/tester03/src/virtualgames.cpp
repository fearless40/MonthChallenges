#include "virtualgames.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <stdexcept>

void VirtualGames::new_game() {
  // Get the last number of guess from last run
  if (m_games.size() > 0) {
    std::size_t reserve = m_games.back().guesses.size();
    m_current.guesses.reserve(reserve);
  } else {
    std::size_t reserve = m_layout.nbrRows.size * m_layout.nbrCols.size + 10;
    m_current.guesses.reserve(reserve);
  }
  m_current.start_time = VirtualGames::ClockT::now();
  if (auto ships = battleship::random_ships(m_layout); ships) {
    m_current.ships = ships.value();
    m_current.hits = make_hits_component(m_current.ships);
  }
  m_current.ending_state = EndingState::none;
}

void VirtualGames::start_guess_timer() {
  m_guess_time = VirtualGames::ClockT::now();
}

void VirtualGames::end_game(VirtualGames::EndingState state) {
  calculate_stats(m_current);
  m_current.ending_state = state;
  m_games.push_back(std::move(m_current));
}

void VirtualGames::calculate_stats(Game &g) {
  if (g.guesses.size() < 1)
    return;

  g.stats.avg_answer = g.guesses.front().elapsed_time;
  for (auto &guess : g.guesses) {
    g.stats.shortest_answer =
        std::min(g.stats.shortest_answer, guess.elapsed_time);
    g.stats.longest_answer =
        std::max(g.stats.longest_answer, guess.elapsed_time);
    g.stats.avg_answer = (g.stats.avg_answer + guess.elapsed_time) / 2;
    g.stats.total_time += guess.elapsed_time;
  }
  g.stats.total_guess_count = g.guesses.size();

  m_global.shortest_answer =
      std::min(m_global.shortest_answer, g.stats.shortest_answer);
  m_global.longest_answer =
      std::max(m_global.longest_answer, g.stats.longest_answer);
  m_global.total_time += g.stats.total_time;
  m_global.average_guess_count =
      (m_global.average_guess_count + g.stats.average_guess_count) / 2;
  m_global.invalid_guess_count =
      (m_global.invalid_guess_count + g.stats.invalid_guess_count) / 2;
  m_global.repeat_guess_count =
      (m_global.repeat_guess_count + g.stats.repeat_guess_count) / 2;
  m_global.avg_answer = (m_global.avg_answer + g.stats.avg_answer) / 2;
};

VirtualGames::GuessResult VirtualGames::guess(const battleship::RowCol guess) {
  auto elapsed_time = VirtualGames::ClockT::now() - m_guess_time;
  m_current.guesses.emplace_back(
      guess, std::chrono::duration_cast<VirtualGames::TimeT>(elapsed_time));

  // See if the guess is valid
  if (!m_layout.is_row_col_valid(guess))
    ++m_current.stats.invalid_guess_count;

  // Find repeats. Can be slow with large numbers of guesses. For modern
  // computers should be no problem
  if (auto exists =
          std::ranges::find(m_current.guesses, guess, &Guess_Stats::guess);
      exists != m_current.guesses.end())
    ++m_current.stats.repeat_guess_count;

  if (auto ship_opt = battleship::ship_at_position(m_current.ships, guess);
      ship_opt) {

    auto shipdef = ship_opt.value().id();
    auto index = m_layout.shipdef_to_index(shipdef);

    if (auto section_opt = m_current.ships[index].ship_section_hit(guess);
        section_opt) {
      m_current.hits[index].hits.set(section_opt.value(), true);
      if (m_current.hits[index].is_sunk()) {
        return {GuessReport::Sink, shipdef};
      } else {
        return {GuessReport::Hit, shipdef};
      }
    }
  }
  return {GuessReport::Miss};
}

VirtualGames::ShipHits
VirtualGames::make_hits_component(battleship::Ships const &ships) {
  ShipHits hits;
  hits.reserve(ships.size());
  battleship::ShipDefinition sid = m_layout.minShipSize;
  for (auto const &ship : ships) {
    hits.emplace_back(std::bitset<32>{}, sid);
    ++sid.size;
  }
  return hits;
}

bool VirtualGames::sunk_all_ships() const {
  for (auto &h : m_current.hits) {
    std::cout << h.id.size << " " << h.is_sunk() << '\n';
  }
  return std::ranges::all_of(m_current.hits,

                             [](auto &hit) { return hit.is_sunk(); });
}
