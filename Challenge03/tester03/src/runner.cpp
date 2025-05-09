#include "runner.hpp"
#include "aistats.hpp"
#include "programoptions.hpp"
#include "reproc++/reproc.hpp"
#include "reprochelper.hpp"
#include "ship.hpp"
#include <bitset>
#include <charconv>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <optional>
#include <system_error>

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
      std::cout << "Total number of AIs found: " << ai_count.value() << '\n';
      ai_ids.resize(ai_count.value());
      std::iota(ai_ids.begin(), ai_ids.end(), 0);
      return ai_ids;
    } else {
      std::cout << "Could not read total number of AIs from application\n";
      return {};
    }
  } else {
    return opt.ai_id_to_test;
  }
  return ai_ids;
}

bool test(ProgramOptions::Options const &opt) {

  auto ai_ids_opt = get_requested_ai(opt);
  if (!ai_ids_opt)
    return false;

  auto ai_ids = ai_ids_opt.value();

  reproc::process subapp;
  reproc::options options{default_process_options()};
  std::array<std::string, 4> cmdline{opt.program_to_test, "run", "--ai", "0"};

  auto ec = subapp.start(cmdline, options);

  if (ec == std::errc::no_such_file_or_directory) {
    std::cout << "Program not found.\n";
    return false;
  }

  unsigned char buffer[256];

  for (int i = 0; i < 5; ++i) {
    std::size_t sz;
    std::tie(sz, ec) =
        subapp.read(reproc::stream::out, (unsigned char *)&buffer, 255);
    stripn(buffer, sz);

    subapp.write((unsigned char *)&"M\n", 2);
  }

  subapp.write((unsigned char *)&"Q\n", 2);

  std::size_t sz;
  std::tie(sz, ec) =
      subapp.read(reproc::stream::out, (unsigned char *)&buffer, 255);
  stripn(buffer, sz);

  options.stop.first = {reproc::stop::wait, reproc::milliseconds(10000)};

  int status = 0;
  std::tie(status, ec) = subapp.stop(options.stop);

  if (ec) {
    std::cout << "Error: " << ec.message() << '\n';
  }

  return true;
}
