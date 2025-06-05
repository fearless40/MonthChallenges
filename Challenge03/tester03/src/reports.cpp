#include "virtualgames.hpp"
#include <chrono>
#include <ostream>
#include <type_traits>

using std::ostream;

struct print_time {
  constexpr explicit print_time(VirtualGames::TimeT const &duration_value)
      : duration(duration_value) {};

  friend std::ostream &operator<<(std::ostream &s, const print_time &pt) {

    using namespace std::chrono_literals;
    if (pt.duration > 1min) {
      s << std::chrono::duration_cast<std::chrono::minutes>(pt.duration);
      return s;
    } else if (pt.duration > 1s) {
      s << std::chrono::duration_cast<std::chrono::seconds>(pt.duration);
      return s;
    } else if (pt.duration > 1ms) {
      s << std::chrono::duration_cast<std::chrono::milliseconds>(pt.duration);
      return s;
    }
    s << pt.duration;
    return s;
  }

private:
  const VirtualGames::TimeT duration;
};

std::ostream &header(std::ostream &s) {
  s << "========================================";
  return s;
};

constexpr std::ostream &operator<<(std::ostream &s, std::byte const b) {
  s << static_cast<std::uint32_t>(b);
  return s;
}
namespace color {

thread_local bool use_color_v = true;

void use_color() { use_color_v = true; }

void no_color() { use_color_v = false; }

struct color {
  constexpr explicit color(unsigned char red, unsigned char green,
                           unsigned char blue)
      : r(red), g(green), b(blue) {};

  friend std::ostream &operator<<(std::ostream &s, const color &c) {
    if (use_color_v)
      s << "\e[38;2;" << c.r << ';' << c.g << ';' << c.b << 'm';
    return s;
  };

private:
  const unsigned char r, g, b;
};

std::ostream &text(std::ostream &s) {
  s << color(255, 255, 255);
  return s;
}

std::ostream &value_normal(std::ostream &s) {
  s << color(200, 200, 200);
  return s;
}

std::ostream &reset(std::ostream &s) {
  s << "\e[0m";
  return s;
}

} // namespace color
// el == EndLine
constexpr std::ostream &el(std::ostream &s) {
  s << '\n';
  return s;
}

void output_game(std::ostream &s, std::size_t id,
                 const VirtualGames::Game &game) {
  s << el << header << el;
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

void print_game_board(std::ostream &s) {}

void print_global_stats(std::ostream &s, const VirtualGames &games,
                        bool use_color) {
  if (use_color)
    color::use_color();
  else
    color::no_color();

  s << color::text << "Total time: " << color::value_normal
    << print_time(games.global_stats().total_time) << el;
  s << color::text << "Invalid Guesses: " << color::value_normal
    << games.global_stats().invalid_guess_count << el;
  s << color::text << "Repeat Guesses: " << color::value_normal
    << games.global_stats().repeat_guess_count << el;
  s << color::text << "Average guess per game: " << color::value_normal
    << games.global_stats().avg_answer << el;

  s << el;
  s << color::text << "Shortest Answer: " << color::value_normal
    << print_time(games.global_stats().shortest_answer) << el;
  s << color::text << "Longest answer: " << color::value_normal
    << print_time(games.global_stats().longest_answer) << el;
  s << color::text << "Average time to answer: " << color::value_normal
    << print_time(games.global_stats().avg_answer) << el;
};
