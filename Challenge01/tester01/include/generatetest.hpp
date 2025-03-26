#pragma once
#include "commandline.hpp"
#include <filesystem>

int generate_tests_cmd_line(std::filesystem::path test_output, CommandLine::TestModes mode, bool huge = false,
                            bool overwrite = false);
