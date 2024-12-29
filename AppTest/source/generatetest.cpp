
#include "generatetest.hpp"
#include "TestConfig.hpp"
#include <iostream>

int generate_tests_cmd_line(std::filesystem::path test_output, CommandLine::TestModes mode, bool huge, bool overwrite)
{
    auto currentPath = std::filesystem::current_path();
    std::cout << "Current Path: " << currentPath << '\n';

    bool noErrors = mode == CommandLine::TestModes::NoErrors;
    Tests::Configuration config{Tests::Configuration::generate_default(noErrors, huge)};

    config.write_all_tests(std::filesystem::current_path(), false);

    return 0;
}
