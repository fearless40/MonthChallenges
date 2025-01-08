#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include "TestDefinition.hpp"
#include "base26.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <ranges>
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>
#include <thread>

using namespace std::chrono_literals;

struct TestResult
{
    std::chrono::milliseconds timeToRun;
    const Tests::Definition &def;
    std::string programOutput;
    bool passed;
};

struct ExecuteResult
{
    std::string data;
    int programReturnCode{0};
    bool programError{false};
    bool timeOut{false};
};

enum class CheckLogType
{
    rejection,
    answer,
    expected,
    program_failure
};

struct CheckLogEntry
{
    CheckLogType type;
    bool foundValue;
    std::string searchValue;
    std::size_t location;
};

ExecuteResult execute_app(const std::vector<std::string> &arguments)
{
    reproc::stop_actions stopActions{{reproc::stop::kill, 5000ms}, {reproc::stop::terminate, 10000ms}, {}};

    std::string output;
    reproc::options options;
    options.stop = stopActions;
    options.deadline = 5000ms;
    reproc::process process;

    std::error_code ec = process.start(arguments, options);

    if (ec == std::errc::no_such_file_or_directory)
    {
        std::cerr << "Program not found. Make sure it's available from the PATH. \n";
        return {"", -1, true, false};
    }

    reproc::sink::string sink(output);

    ec = reproc::drain(process, sink, reproc::sink::null);
    if (ec)
    {
        std::cout << "Drain error: " << ec.message() << '\n';
        return {output, 0, false, false};
    }

    int status = 0;
    std::tie(status, ec) = process.wait(reproc::infinite);
    if (ec && ec.value() != static_cast<int>(std::errc::operation_not_permitted))
    {
        std::cout << "Stop error: " << ec.message() << " " << ec.value() << '\n';
        std::cout << "Unable to get program exit code.\n";
        std::cout << "Program error code: " << status << '\n';
    }

    for (auto tok : std::views::split(output, '\n'))
    {
        std::cout << " >> " << std::string_view(tok.begin(), tok.end()) << '\n';
    }

    return {output, 0, false, false};
}

bool program_output_pass(const Tests::Configuration::ExpectedResults &expected, const ExecuteResult &exeResults)
{

    if (exeResults.programError || exeResults.timeOut)
        return false;

    std::string lowerProgramOutput;
    lowerProgramOutput.reserve(exeResults.data.size());
    std::transform(exeResults.data.begin(), exeResults.data.end(), std::back_inserter(lowerProgramOutput),
                   base26::toLower);

    // Look if the rejected keyword pops up in the input
    if (expected.rejected.size() > 0)
    {
        if (std::none_of(expected.rejected.begin(), expected.rejected.end(), [&](const auto &lookFor) {
                std::string lcase;
                lcase.reserve(lookFor.size());
                std::transform(lookFor.begin(), lookFor.end(), std::back_inserter(lcase), base26::toLower);

                auto result = lowerProgramOutput.find(lcase, 0);
                if (result == std::string::npos)
                {
                    return true;
                }
                std::cout << std::format("found rejected string {} at {}\n", lookFor, result);
                return false;
            }))
        {
            std::cout << "Failed due to finding rejected string.\n";
            return false;
        }
    }

    if (expected.test.mError != Tests::Errors::None)
    {
        // Search for the word ERROR
        auto result = lowerProgramOutput.find("error");
        if (result == std::string::npos)
        {
            return false;
        }
        return true;
    }

    // Look for the answers now
    // Generate answers to search for
    std::vector<std::string> answers;
    answers.reserve(expected.expected.size());
    std::transform(expected.expected.begin(), expected.expected.end(), std::back_inserter(answers),
                   [](const Tests::QueryAnswer &q) {
                       if (q.is_oob)
                           return std::string("oob");
                       return std::to_string(q.answer);
                   });

    return std::all_of(answers.begin(), answers.end(), [&](const auto &lookFor) {
        std::string lcase;
        lcase.reserve(lookFor.size());
        std::transform(lookFor.begin(), lookFor.end(), std::back_inserter(lcase), base26::toLower);

        auto result = lowerProgramOutput.find(lcase, 0);
        if (result == std::string::npos)
        {
            std::cout << std::format("Did not find expected result: {} \n", lookFor);
            return false;
        }
        std::cout << std::format("Found result {} at {}\n", lookFor, result);
        return true;
    });
}

TestResult run_test(const Tests::Configuration::ExpectedResults &expected, std::string &app)
{

    std::vector<std::string> cmdVector{app, "--load", expected.filename, "--guess"};

    for (auto &guess : expected.queries)
    {
        cmdVector.push_back(guess.as_base26_fmt());
    }
    if (expected.queries.size() == 0)
    {
        cmdVector.push_back("a0");
    }

    std::string cmd = std::accumulate(std::next(cmdVector.begin()), cmdVector.end(), cmdVector[0],
                                      [](std::string a, std::string &b) { return std::move(a) + ' ' + b; });

    std::cout << "Running test: " << cmd << '\n';
    auto start = std::chrono::high_resolution_clock::now();
    auto ret = execute_app(cmdVector);
    auto end = std::chrono::high_resolution_clock::now();

    TestResult result{std::chrono::duration_cast<std::chrono::milliseconds>(end - start), expected.test, ret.data,
                      program_output_pass(expected, ret)};

    std::cout << std::format("Passed: [{}] Took: [{}] \n", result.passed, result.timeToRun);
    return result;
}

std::vector<TestResult> run_all_tests(Tests::Configuration &config, std::filesystem::path app)
{

    std::vector<TestResult> results;
    std::string appString = app.generic_string();
    for (auto const &expected : config)
    {
        results.push_back(run_test(expected, appString));
    }

    return results;
}

int main_run_tests(std::filesystem::path testPath, std::filesystem::path exe)
{

    Tests::Configuration loadMe;

    loadMe = Tests::toml_file_to_config(testPath);

    std::cout << "Finished Loading: " << testPath << '\n';

    run_all_tests(loadMe, exe);

    return 0;
}