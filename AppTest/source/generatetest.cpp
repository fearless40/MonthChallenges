
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
#include "TestConfig.hpp"

using std::operator""sv;

int generate_tests_cmd_line(std::filesystem::path test_output, CommandLine::TestModes mode, bool huge, bool overwrite)
{
    auto currentPath = std::filesystem::current_path();
    std::cout << "Current Path: " << currentPath << '\n';
    
    bool noErrors = mode == CommandLine::TestModes::NoErrors;
    Tests::Configuration config { Tests::Configuration::generate_default( noErrors, huge )};

    
    config.write_all_tests(std::filesystem::current_path(), false);

    return 0;
}
