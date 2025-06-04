#include "virtualgames.hpp"
#include <chrono>
#include <ostream>
#include <type_traits>

std::ostream &print_time(std::ostream &s, VirtualGames::TimeT const duration) {
  using namespace std::chrono_literals;
  if (duration > 1min) {
    s << std::chrono::duration_cast<std::chrono::minutes>(duration);
    return s;
  } else if (duration > 1s) {
    s << std::chrono::duration_cast<std::chrono::seconds>(duration);
    return s;
  } else if (duration > 1ms) {
    s << std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return s;
  }
  s << duration;
  return s;
}

std::ostream &header(std::ostream &s) {
  s << "========================================";
  return s;
};

void output_game(std::ostream &s, std::size_t id,
                 const VirtualGames::Game &game) {
  constexpr char e = '\n';
  s << e << output_header() << e;
  s << "Game Number: " << id << e;
  s << e;
  output_time(s, "Total time", game.stats.total_time);

  s << "Total Guesses: " << game.stats.total_guess_count << e;
  s << "Invalid Guesses: " << game.stats.invalid_guess_count << e;
  s << "Repeat Guesses: " << game.stats.repeat_guess_count << e;
  s << e;
  s << "Shortest Answer: " << game.stats.shortest_answer << e;
  output_time(s, "Longest answer", game.stats.longest_answer);
  s << "Average Answer:" << game.stats.avg_answer << e;
}

void output_games(std::ostream &s,
                  const std::vector<VirtualGames::Game> &games) {
  std::size_t game_count = 0;
  for (auto const &game : games) {
    output_game(s, ++game_count, game);
  }
}

void output_report(std::ostream &s, const VirtualGames &games, bool use_color) {
  const char e = '\n';

  s << "Program: " << games.program_name() << e;
  s << "AI ID: " << games.aiid() << e;
  s << e;
  output_time(s, "Total time", games.global_stats().total_time);
  // s << "Total time: " << games.global_stats().total_time << e;
  s << "Invalid Guesses: " << games.global_stats().invalid_guess_count << e;
  s << "Repeat Guesses: " << games.global_stats().repeat_guess_count << e;
  s << "Average guess per game: " << games.global_stats().average_guess_count
    << e;
  s << e;
  s << "Shortest Answer: " << games.global_stats().shortest_answer << e;
  output_time(s, "Longest answer", games.global_stats().longest_answer);
  s << "Average Answer: " << games.global_stats().avg_answer << e;
};
