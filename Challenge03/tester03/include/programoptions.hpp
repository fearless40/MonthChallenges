#pragma once

#include <cstddef>
#include <string>
#include <vector>
namespace ProgramOptions {

enum class RunMode { test, help, version, error };

struct Options {
  RunMode mode{RunMode::error};
  std::size_t nbrIterations{100};
  std::size_t rowSize{10};
  std::size_t colSize{10};
  std::size_t smallestShip{2};
  std::size_t largestShip{5};
  std::size_t wait_upto_millis{500};
  std::string program_to_test{};
  std::string ship_layout_file{};
  std::string result_file{"results.txt"};
  std::string options_file{};

  bool randomShips{true};
  bool display_histogram{false};
  bool all_ai{false};

  std::vector<std::size_t> ai_id_to_test{};
};
} // namespace ProgramOptions
