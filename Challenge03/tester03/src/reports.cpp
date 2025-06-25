#include "baseconv.hpp"
#include "virtualgames.hpp"
#include <chrono>
#include <cstddef>
#include <ostream>
namespace report {
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

struct repeat {
  constexpr explicit repeat(std::size_t count, const std::string_view &value)
      : m_count(count), m_value(value) {}

  friend std::ostream &operator<<(std::ostream &s, const repeat &rp) {
    for (std::size_t c = 0; c < rp.m_count; ++c) {
      s << rp.m_value;
    }
    return s;
  }

private:
  const std::size_t m_count;
  const std::string_view m_value;
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
    if (use_color_v) {
      s << "\e[38;2;" << static_cast<int>(c.r) << ";" << static_cast<int>(c.g)
        << ";" << static_cast<int>(c.b) << "m";
    }
    return s;
  };

private:
  const unsigned char r, g, b;
};

std::ostream &text(std::ostream &s) {
  s << color(200, 200, 200);
  return s;
}

std::ostream &value_normal(std::ostream &s) {
  s << color(255, 255, 255);
  return s;
}

std::ostream &value_abnormal(std::ostream &s) {
  s << color(220, 20, 20);
  return s;
}

std::ostream &highlite(std::ostream &s) {
  s << color(0, 220, 0);
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

void print_single_game_stats(std::ostream &s, std::size_t id,
                             const VirtualGames::Game &game) {
  s << color::text << "Game Number: " << color::value_normal << id << el;
  s << color::text << "Total time: " << color::value_normal
    << game.stats.total_time << el;

  s << color::text << "Total Guesses: " << color::value_normal
    << game.stats.total_guess_count << el;
  s << color::text << "Invalid Guesses: " << color::value_normal
    << game.stats.invalid_guess_count << el;
  s << color::text << "Repeat Guesses: " << color::value_normal
    << game.stats.repeat_guess_count << el;
  s << color::text << "Shortest Answer: " << color::value_normal
    << game.stats.shortest_answer << el;
  s << color::text << "Longest answer: " << color::value_normal
    << game.stats.longest_answer << el;
  s << color::text << "Average Answer: " << color::value_normal
    << game.stats.avg_answer << el;
}

// void output_games(std::ostream &s,
//                   const std::vector<VirtualGames::Game> &games) {
//   std::size_t game_count = 0;
//   for (auto const &game : games) {
//     output_game(s, ++game_count, game);
//   }
// }
//
void print_game_board(std::ostream &s, battleship::GameLayout const &layout,
                      battleship::Ships const &ships) {
  // Write header
  //    A B C D E F G H I J K L
  //    _______________
  //  1|. 2 . . . . . . . . . .
  //   |
  //  2|
  //   |
  //  3|
  //   |
  // 10|

  s << "    " << color::text; // two blank spaces for header
  for (std::size_t col = 0; col != layout.nbrCols.size; ++col) {
    s << base26::to_string(static_cast<int>(col)) << ' ';
  }
  s << el << color::text;

  // Col line
  s << "   " << "┌" << repeat(layout.nbrCols.size * 2, "─") << el;

  // Rows
  for (std::size_t row = 0; row < layout.nbrRows.size; ++row) {
    if (row < 10)
      s << "  ";
    else
      s << " ";
    s << row << "│";
    for (std::size_t col = 0; col < layout.nbrCols.size; ++col) {
      if (auto ship = battleship::ship_at_position(
              ships,
              battleship::RowCol{
                  battleship::Row{static_cast<unsigned short>(row)},
                  battleship::Col{static_cast<unsigned short>(col)}});
          ship) {
        s << color::highlite << ship.value().id().size;
      } else {
        s << color::color(80, 80, 80) << ".";
      }
      s << color::reset << " ";
    }
    s << el;
  }
}
void print_colors_on() { color::use_color(); }

void print_colors_off() { color::no_color(); }

// Only prints the first game for now
void print_all_moves(std::ostream &s, const VirtualGames::Game &game) {
  auto int_size = [](std::size_t i) -> size_t {
    if (i < 10)
      return 1;
    if (i < 100)
      return 2;
    if (i < 1000)
      return 3;
    if (i < 10000)
      return 4;
    return 0;
  };

  s << "ID "
    << "Move" << '\n';
  std::size_t id = 0;
  std::size_t col = 0;
  std::size_t maxcolsize = 2;
  if (game.guesses.size() > 99) {
    maxcolsize = 3;
  } else if (game.guesses.size() > 999) {
    maxcolsize = 4;
  }

  for (auto &move : game.guesses) {
    s << ++id;
    s << repeat(maxcolsize - int_size(id), " ");
    s << " ";
    switch (move.result) {
    case VirtualGames::Guess_Stats_Result::repeat:
      s << color::color(200, 200, 0);
      break;
    case VirtualGames::Guess_Stats_Result::invalid:
      s << color::color(200, 0, 0);
      break;
    case VirtualGames::Guess_Stats_Result::hit:
      s << color::color(0, 200, 0);
      break;
    case VirtualGames::Guess_Stats_Result::miss:
      s << color::value_normal;
      break;
    case VirtualGames::Guess_Stats_Result::sunk:
      s << color::color(90, 100, 250);
      break;
    case VirtualGames::Guess_Stats_Result::unknown:
      s << color::color(0, 0, 255);
      break;
    }

    s << move.guess.as_base26_fmt() << ":" << (int)move.result;
    s << ':' << move.elapsed_time << "\t\t";
    if (++col > 3) {
      s << '\n';
      col = 0;
    }
    s << color::value_normal;
  }
}

void print_global_stats(std::ostream &s, const VirtualGames &games) {

  s << color::text << "Total time: " << color::value_normal
    << print_time(games.global_stats().total_time) << el;
  s << color::text << "Invalid Guesses: " << color::value_normal
    << games.global_stats().invalid_guess_count << el;
  s << color::text << "Repeat Guesses: " << color::value_normal
    << games.global_stats().repeat_guess_count << el;
  s << color::text << "Average guess per game: " << color::value_normal
    << games.global_stats().average_guess_count << el;

  s << el;
  s << color::text << "Shortest Answer: " << color::value_normal
    << print_time(games.global_stats().shortest_answer) << el;
  s << color::text << "Longest answer: " << color::value_normal
    << print_time(games.global_stats().longest_answer) << el;
  s << color::text << "Average time to answer: " << color::value_normal
    << print_time(games.global_stats().avg_answer) << el;

  s << color::text << "Count of games 'timed out': " << color::value_normal
    << games.global_stats().ending_state(VirtualGames::EndingState::timeout)
    << el;
  s << color::text << "Count of games 'program_error': " << color::value_normal
    << games.global_stats().ending_state(
           VirtualGames::EndingState::program_error)
    << el;
  s << color::text << "Count of games 'other': " << color::value_normal
    << games.global_stats().ending_state(VirtualGames::EndingState::other)
    << el;
  s << color::text << "Count of games 'too_many_guess': " << color::value_normal
    << games.global_stats().ending_state(
           VirtualGames::EndingState::too_many_guess)
    << el;
  s << color::text
    << "Count of games 'unable_read_output: " << color::value_normal
    << games.global_stats().ending_state(
           VirtualGames::EndingState::unable_read_output)
    << el;
  s << color::text
    << "Count of games 'program_has_no_guesses': " << color::value_normal
    << games.global_stats().ending_state(
           VirtualGames::EndingState::program_has_no_guesses)
    << el;
  s << color::text << "Count of games 'unknown': " << color::value_abnormal
    << games.global_stats().ending_state(VirtualGames::EndingState::none) << el;

  s << color::text << "Count of games 'sunk_all_ships': " << color::value_normal
    << games.global_stats().ending_state(
           VirtualGames::EndingState::sunk_all_ships)
    << el;
};
} // namespace report
