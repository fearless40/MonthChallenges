#include "virtualgames.hpp"

void VirtualGames::new_game() {
  // Get the last number of guess from last run
  if (m_games.size() > 0) {
    std::size_t reserve = m_games.back().guesses.size();
    m_current.guesses.reserve(reserve);
  } else {
    std::size_t reserve = m_layout.nbrRows.size * m_layout.nbrCols.size + 10;
    m_current.guesses.reserve(reserve);
  }
}
