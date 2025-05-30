#include "runner.hpp"
#include "aistats.hpp"
#include "programoptions.hpp"
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
bool test(ProgramOptions::Options const &opt) {

  auto ai_ids_opt = get_requested_ai(opt);

  if (!ai_ids_opt)
    return false;

  auto &ai_ids = ai_ids_opt.value();

  TestRunner tester{opt, ai_ids.front()};
  std::atomic<bool> done_thread{false};

  // tester.start_tests(opt.nbrIterations);

  auto thread_me = [&tester, &done_thread](std::size_t iterations) {
    std::cout << std::this_thread::get_id() << " Run Tests " << '\n';
    tester.start_tests(iterations);
    done_thread = true;
  };

  std::jthread run_test{thread_me, opt.nbrIterations};

  std::cout << std::this_thread::get_id() << " Main function " << '\n';
  while (done_thread == false) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Waiting..." << '\n';
  }
  run_test.join();

  if (opt.result_file != "") {
    output_report(std::cout, tester.games());
  }

  return true;
}

void output_time(std::ostream &s, const char *label,
                 VirtualGames::TimeT duration) {
  if (label)
    s << label << ": ";

  if (duration.count() > 1000) {
    s << std::chrono::duration_cast<std::chrono::milliseconds>(duration)
      << '\n';
  } else {
    s << duration << '\n';
  }
}

void output_report(std::ostream &s, const VirtualGames &games) {
  const char *header = "====================================================\n";
  const char e = '\n';

  std::size_t game_count = 0;
  for (auto const &game : games.all_games()) {
    s << e << header << e;
    s << "Game Number: " << ++game_count << e;
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
  s << "Testing Results\n" << header;
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
  s << "Average Answer:" << games.global_stats().avg_answer << e;
};
