
#include "generatetest.hpp"
#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <ranges>
#include <sstream>
#include <string_view>
#include <variant>

using std::operator""sv;

int generate_tests_cmd_line(std::filesystem::path test_output, CommandLine::TestModes mode, bool huge, bool overwrite)
{
    auto currentPath = std::filesystem::current_path();
    std::cout << "Current Path: " << currentPath << '\n';
    TestsConfiguration config;

    std::ranges::for_each(tests | std::views::filter([&mode](auto &t) {
                              if (mode == CommandLine::TestModes::NoErrors)
                                  return t.mError == Errors::None;
                              return true;
                          }),
                          [&config, &currentPath](const TestDefinition &test) {
                              config.create(test, 5, 2);
                              // make_test(config, test, currentPath);
                          });

    config.write_all_tests(std::filesystem::current_path(), false);

    return 0;
}
