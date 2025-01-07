#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include "TestDefinition.hpp"
#include "subprocess.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

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

ExecuteResult execute_app(const std::string &arguments)
{
    namespace sp = subprocess;
    auto result = sp::Popen(arguments, sp::output{sp::PIPE}, sp::bufsize{2048});
    std::string appResult;
    std::atomic<bool> programFinished{false};
    /* Read pipe until end of file, or an error occurs. */
    // If it takes longer than 1 min then quit with a time out.
    auto start = std::chrono::high_resolution_clock::now();

    std::jthread runSubprocess([&]() {
        appResult = result.communicate().first.buf.data();
        programFinished = true;
    });

    while (!programFinished && ((std::chrono::high_resolution_clock::now() - start) < std::chrono::milliseconds(500)))
    {
    }
    if (!programFinished)
    {
        std::cout << "Attempting to kill the application \n";
        result.kill();
        return {appResult, result.retcode(), false, true};
    }
    std::cout << appResult << '\n';
    return {appResult, result.retcode(), false, false};
}

bool program_output_pass(const Tests::Configuration::ExpectedResults &expected, const ExecuteResult &exeResults)
{

    if (exeResults.programError || exeResults.timeOut)
        return false;

    // Look if the rejected keyword pops up in the input
    if (expected.rejected.size() > 0)
    {
        if (!std::all_of(expected.rejected.begin(), expected.rejected.end(), [&](const auto &lookFor) {
                auto result = exeResults.data.find(lookFor, 0);
                if (result != std::string::npos)
                    return false;
                return true;
            }))
            return false;
    }

    // Look for the answers now
    // Generate answers to search for
    std::vector<std::string> answers;
    for (const Tests::QueryAnswer &q : expected.expected)
    {
        if (q.is_oob)
        {
            answers.emplace_back("OOB");
        }
        else
        {
            answers.emplace_back(std::format("{}", q.answer));
        }
    }

    return std::all_of(answers.begin(), answers.end(), [&](const auto &lookFor) {
        auto result = exeResults.data.find(lookFor, 0);
        if (result != std::string::npos)
            return false;
        return true;
    });
}

TestResult run_test(const Tests::Configuration::ExpectedResults &expected, std::string &app)
{

    std::string guesses;
    guesses.reserve(512);

    for (auto &guess : expected.queries)
    {
        guesses.append(guess.as_base26_fmt());
        guesses.append(" ");
    }
    if (expected.queries.size() == 0)
    {
        guesses.append("a0");
    }

    const std::string cmd = std::format("{} --load {} --guess {}", app, expected.filename, guesses);
    std::cout << "Running test: " << cmd << '\n';
    auto start = std::chrono::high_resolution_clock::now();
    auto ret = execute_app(cmd);
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