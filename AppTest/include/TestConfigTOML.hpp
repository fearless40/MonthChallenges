#pragma once
#include "TestConfig.hpp"
#include <filesystem>

namespace Tests
{
bool config_to_toml_file(const Configuration &config, std::filesystem::path filename);
Configuration toml_file_to_config(std::filesystem::path filename);
}