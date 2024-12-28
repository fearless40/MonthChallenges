
#pragma once
#include "TestDefinition.hpp"
#include <filesystem>

namespace Tests
{

class Configuration
{
  public:
    struct ExpectedResults
    {
        const Definition test;
        Queries queries;
        QueryAnswers expected;
        std::vector<std::string> rejected;
    };

  private:
    std::vector<ExpectedResults> mTests;

  public:
    void create(const Definition &test, std::size_t nbrQueries, std::size_t outofbounds);

    void write_all_tests(std::filesystem::path locationToWrite, bool locateTestFileInSeperateFolder);

  private:
    void write_config_file(std::filesystem::path locationToWrite);
};
} // namespace Tests