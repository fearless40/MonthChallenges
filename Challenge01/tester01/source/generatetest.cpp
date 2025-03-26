
#include "generatetest.hpp"
#include "TestConfig.hpp"
#include "TestConfigTOML.hpp"
#include <iostream>

int generate_tests_cmd_line(std::filesystem::path test_output, CommandLine::TestModes mode, bool huge, bool overwrite)
{
    auto currentPath = std::filesystem::current_path();
    std::cout << "Current Path: " << currentPath << '\n';
    std::cout << "Writing test definition file to: " << test_output << '\n';

    bool noErrors = mode == CommandLine::TestModes::NoErrors;
    Tests::Configuration config{Tests::Configuration::generate_default(noErrors, huge)};

    config.write_all_tests(std::filesystem::current_path(), false);

    Tests::config_to_toml_file(config, test_output);

    return 0;
}
