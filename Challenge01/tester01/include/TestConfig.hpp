
#pragma once
#include "TestDefinition.hpp"
#include <filesystem>

namespace Tests {

class Configuration {
public:
  struct ExpectedResults {
    const Definition test;
    std::string filename{};
    Queries queries{};
    QueryAnswers expected{};
    std::vector<std::string> rejected{};
  };

private:
  std::vector<ExpectedResults> mTests;

public:
  /***
   * @details Creates a new test from scratch, indicating that data will be
   * synthesized
   * @param test Create the test based on a definition
   * @param nbrQueries specify the number of queries that are required
   * @param outofbounds specify the number of guarenteed out of bound queries
   * are synthesize
   */
  void create_new_test(const Definition &test, std::size_t nbrQueries,
                       std::size_t outofbounds);

  /***
   * @details Defines an existing test create
   */
  void add_existing(const Definition &test, std::string &&filename, Queries &&,
                    QueryAnswers &&, std::vector<std::string> &&);

  void write_all_tests(std::filesystem::path locationToWrite,
                       bool locateTestFileInSeperateFolder);

  auto begin() const { return mTests.cbegin(); }

  auto end() const { return mTests.cend(); }

public:
  static Configuration generate_default(bool includeErrors, bool generateHuge);

private:
  void write_config_file(std::filesystem::path locationToWrite);
};
} // namespace Tests
