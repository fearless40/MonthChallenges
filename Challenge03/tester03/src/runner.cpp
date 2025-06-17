#include "runner.hpp"
#include "aistats.hpp"
#include "programoptions.hpp"
#include "reports.hpp"
#include "reproc++/reproc.hpp"
#include "reprochelper.hpp"
#include "testrunner.hpp"
#include "virtualgames.hpp"
#include <charconv>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <optional>
#include <ostream>
#include <system_error>
#include <thread>
#include <vector>

const size_t MAX_ITERATIONS_PER_THREAD = 1500;

void stripn(unsigned char *buff, std::size_t size) {
  for (std::size_t i = 0; i < size; ++i) {
    if (buff[i] != '\n')
      std::cout << buff[i];
  }
  std::cout << ',';
}

;

std::optional<std::size_t>
get_ai_number_from_app(ProgramOptions::Options const &opt) {
  reproc::process app;
  reproc::options options{default_process_options()};

  std::array<std::string, 2> cmdline{opt.program_to_test, "ai"};
  std::error_code ec = app.start(cmdline, options);

  if (ec == std::errc::no_such_file_or_directory)
    return {};

  unsigned char buff[255];
  std::size_t bytes_read;
  std::tie(bytes_read, ec) =
      app.read(reproc::stream::out, (unsigned char *)&buff, 254);

  options.stop.first = {reproc::stop::wait, reproc::milliseconds(1000)};
  app.stop(options.stop);
  if (bytes_read != 0) {
    std::size_t nbrAi{0};
    auto result =
        std::from_chars((char *)buff, (char *)(buff + bytes_read), nbrAi);
    if (result.ptr != (char *)buff) {
      return nbrAi;
    }
  }

  return {};
}

std::optional<std::vector<AIID>>
get_requested_ai(ProgramOptions::Options const &opt) {
  std::vector<std::size_t> ai_ids;

  // Get all the AIs that the user wants to test
  if (opt.all_ai) {
    auto ai_count = get_ai_number_from_app(opt);
    if (ai_count) {
      // std::cout << "Total number of AIs found: " << ai_count.value() << '\n';
      ai_ids.resize(ai_count.value());
      std::iota(ai_ids.begin(), ai_ids.end(), 0);
      return ai_ids;
    } else {
      std::cout << "Could not read total number of AIs from application\n";
      return {};
    }
  } else {
    // std::cout << "Test AI: " << opt.ai_id_to_test << '\n';
    // std::cout << "Total number of ais: " << opt.ai_id_to_test.size() << '\n';
    return opt.ai_id_to_test;
  }
  return {};
}

void output_report(std::ostream &s, const VirtualGames &game);
void output_games(std::ostream &s,
                  const std::vector<VirtualGames::Game> &games);
struct color {
  constexpr explicit color(unsigned char red, unsigned char green,
                           unsigned char blue)
      : r(red), g(green), b(blue) {};

  friend std::ostream &operator<<(std::ostream &s, const color &c) {
    // std::cout << "\e[38;2;" << i + 50 << ";" << 100 << ";" << i * 2 +
    s << "\e[38;2;" << static_cast<int>(c.r) << ";" << static_cast<int>(c.g)
      << ";" << static_cast<int>(c.b) << "m";
    return s;
  };

private:
  const unsigned char r, g, b;
};

struct ScheduledTest {
  TestRunner runner;
  std::size_t iterations;
};

// std::vector<ScheduledTest> make_test_tasks(ProgramOptions::Options const &
// opt,  std::vector<AIID> const &ids) {
//
//   std::vector<TestRunner> schedule;
//   std::size_t extra_tasks = 1;
//   if (opt.nbrIterations > MAX_ITERATIONS_PER_THREAD) {
//     auto tasks_division = std::div(iterations, MAX_ITERATIONS_PER_THREAD);
//     for (std::size_t count = 0; count < tasks_division; ++count) {
//          schedule.emplace_back(TestRunner{opt, id)
//

bool test(ProgramOptions::Options const &opt) {

  std::string_view const bar = "▒";
  auto ai_ids_opt = get_requested_ai(opt);

  if (!ai_ids_opt)
    return false;

  auto &ai_ids = ai_ids_opt.value();

  std::vector<TestRunner> tests;

  for (auto ai_id : ai_ids) {
    tests.emplace_back(opt, ai_id);
  };

  std::atomic<bool> done_thread{false};

  auto thread_me = [](std::vector<TestRunner>::iterator it,
                      std::size_t iterations) { it->start_tests(iterations); };

  std::vector<std::jthread> threads;

  for (auto it = tests.begin(); it != tests.end(); ++it) {

    threads.emplace_back(thread_me, it, opt.nbrIterations);
  }

  std::cout << '\n';
  // std::cout << "\e[H";
  std::cout << tests.front().games().program_name() << '\n';
  while (done_thread == false) {

    std::size_t nbr_finished = 0;
    // Now write the AI
    for (auto &it : tests) {
      std::cout << "\e[0K" << "\e[0m"; // Erase to end of line
      std::cout << it.games().aiid() << " :: ";
      if (it.is_completed()) {
        std::cout << "\e[38;2;0;220;0m" << "done";
        ++nbr_finished;
      } else {
        auto current_round = it.current_round();
        std::cout << "\e[0m" << std::setw(5) << current_round << " :: ";
        auto percent = ((current_round * 100) / opt.nbrIterations) / 4;
        std::cout << std::setw(3) << percent * 4 << "% :: ";
        for (std::size_t i = 1; i < percent; ++i) {

          std::cout << "\e[38;2;" << i + 50 << ";" << 100 << ";" << i * 2 + 50
                    << "m" << bar;
          // std::cout << color(i + 50, 100, i * 2 + 50) << bar;
        }
      }

      std::cout << '\n';
    }

    if (nbr_finished >= tests.size()) {
      done_thread = true;
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "\e[" << tests.size() << 'F';
  }

  for (auto &thread : threads) {
    thread.join();
  }

  if (opt.result_file == "") {
    for (auto const &runner : tests) {
      std::cout << "\e[0m";
      // std::cout << output_header() << '\n';
      output_report(std::cout, runner.games());
    }
  } else {
    // std::ofstream file{opt.result_file, std::ios::trunc};
    // if (file) {
    //   std::cout << "Writing report to: " << opt.result_file << '\n';
    //   auto const time = std::chrono::current_zone()->to_local(
    //       std::chrono::system_clock::now());
    //   file << "Testing report on " << std::format("{:%m-%d-%Y %X}",
    //   time)
    //        << '\n';
    //   for (auto const &runner : tests) {
    //     file << output_header() << '\n';
    //     output_report(file, runner.games());
    //   }
    //   file << output_header() << "\nGames:\n";
    //   for (auto const &runner : tests) {
    //     output_games(file, runner.games().all_games());
    //   }
    //
    //   file << '\n' << output_header() << "\nMoves:\n";
    //   // for( auto const & runner : test) {
    //   //   output_moves(file, runner.games() );
    //   // }
    // }
  }

  return true;
}

void output_report(std::ostream &s, VirtualGames const &games) {
  report::print_colors_on();
  report::print_global_stats(s, games);
  report::print_game_board(s, games.layout(), games.all_games().front().ships);
}
