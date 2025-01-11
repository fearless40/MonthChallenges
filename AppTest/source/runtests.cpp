#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include "TestDefinition.hpp"
#include "stringutil.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <ranges>
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>
#include <thread>
#include <variant>

using namespace std::chrono_literals;

struct AppRejection
{
    std::string rejectionText;
    std::size_t location;
    operator std::string() const noexcept
    {
        return as_string();
    }
    std::string as_string() const noexcept
    {
        return std::format("Found rejection text [{}] at location [{}]'\n", rejectionText, location);
    }
};

struct AppMissingAnswer
{
    std::string missingAnswer;
    operator std::string() const noexcept
    {
        return as_string();
    }
    std::string as_string() const noexcept
    {
        return std::format("Missing answer [{}] '\n", missingAnswer);
    }
};

struct AppFoundAnswer
{
    std::string answer;
    std::size_t location;
    operator std::string() const noexcept
    {
        return as_string();
    }
    std::string as_string() const noexcept
    {
        return std::format("Found answer [{}] at location [{}]'\n", answer, location);
    }
};

struct AppTimeOut
{
    std::chrono::milliseconds timeout;
    operator std::string() const noexcept
    {
        return as_string();
    }

    std::string as_string() const noexcept
    {
        return std::format("Application timed out after [{}] '\n", timeout);
    }
};

struct AppErrorCondition
{
    std::error_code ec;
    operator std::string() const noexcept
    {
        return as_string();
    }

    std::string as_string() const noexcept
    {
        return std::format("Error in running app with code [{}] with message [{}] '\n", ec.value(), ec.message());
    }
};

using AppLogEntry = std::variant<AppRejection, AppMissingAnswer, AppFoundAnswer, AppTimeOut, AppErrorCondition>;
using AppLog = std::vector<AppLogEntry>;

struct TestResult
{
    const Tests::Definition &def;
    std::chrono::milliseconds timeToRun;
    std::string programOutput;
    std::string cmdline;
    std::vector<AppLogEntry> logs;
    bool passed;

    TestResult(const Tests::Definition &definition) : def(definition) {};

  private:
    TestResult();
};

struct ExecuteResult
{
    bool failed{true};
    std::string appOutput;
    int appReturnCode{-1};
    std::vector<AppLogEntry> logs;
};

ExecuteResult execute_app(const std::vector<std::string> &arguments)
{
    reproc::stop_actions stopActions{{reproc::stop::kill, 100ms}, {reproc::stop::terminate, 1000ms}, {}};

    ExecuteResult exeResult;

    std::string output;
    reproc::options options;
    options.stop = stopActions;
    options.deadline = 100ms;
    reproc::process process;

    std::error_code ec = process.start(arguments, options);

    if (ec == std::errc::no_such_file_or_directory)
    {
        std::cerr << "Program not found. Make sure it's available from the PATH. \n";
        exeResult.logs.push_back(AppErrorCondition(ec));
        return exeResult;
    }

    reproc::sink::string sink(exeResult.appOutput);

    ec = reproc::drain(process, sink, reproc::sink::null);
    if (ec == std::errc::timed_out)
    {
        exeResult.logs.push_back(AppTimeOut{5000ms});
        return exeResult;
    }
    else if (ec)
    {
        std::cerr << "Drain error: " << ec.message() << '\n';
        exeResult.logs.push_back(AppErrorCondition(ec));
        return exeResult;
    }

    int status = 0;
    std::tie(status, ec) = process.wait(reproc::infinite);

    if (ec && ec.value() != static_cast<int>(std::errc::operation_not_permitted))
    {
        std::cerr << "Stop error: " << ec.message() << " " << ec.value() << '\n';
        std::cerr << "Unable to get program exit code.\n";
        std::cerr << "Program error code: " << status << '\n';
        exeResult.logs.push_back(AppErrorCondition(ec));
        return exeResult;
    }

    /*for (auto tok : std::views::split(output, '\n'))
    {
        std::cout << " >> " << std::string_view(tok.begin(), tok.end()) << '\n';
    }*/

    exeResult.failed = false;
    exeResult.appReturnCode = status;

    return exeResult;
}

bool verify_whole_string(const std::string &output, const std::string &foundit, std::size_t startingPos)
{
    // looking for 3:  123 [bad]// 321 [bad]// 12,-3, [bad]// 1 2 3 [ok] // 1,2,3 [ok]//  (12)
    const std::string_view allowedSeperators{", []();|\n\r\t\0"};
    std::size_t startOfString = startingPos;
    std::size_t endOfString = startingPos + foundit.length();

    // Search toward the begin() of the string
    for (std::size_t i = startingPos; i >= 0; --i)
    {
        // std::cout << output[i] << " " << i << '\n';
        auto res = allowedSeperators.find_first_of(output[i]);
        if (res != std::string_view::npos || i == 0)
        {
            // std::cout << "res != npos " << output[i] << " " << i << '\n';
            startOfString = i != 0 ? i + 1 : i;
            break;
        }
    }

    // Search to the end() of the string
    for (std::size_t i = startingPos; i <= output.length(); ++i)
    {
        auto res = allowedSeperators.find_first_of(output[i]);
        if (res != std::string_view::npos || i == output.length())
        {
            endOfString = i;
            break;
        }
    }

    /*std::cout << "------------------------------------------\n";
    std::cout << output << '\n';
    std::cout << std::format("Verfiy:: start:{} to end:{} is {} == {}'\n", startOfString, endOfString,
                             output.substr(startOfString, (endOfString - startOfString)), foundit);
    std::cout << "Verify: " << output.substr(startOfString, endOfString - startOfString) << " == " << foundit << '\n';
    std::cout << (output.substr(startOfString, (endOfString - startOfString)) == foundit) << '\n';*/
    return output.substr(startOfString, (endOfString - startOfString)) == foundit;
}

bool program_output_pass(const Tests::Configuration::ExpectedResults &expected, const std::string &appOutput,
                         std::vector<AppLogEntry> &log)
{

    std::string lowerProgramOutput = util::to_lower_copy(appOutput);

    // Look if the rejected keyword pops up in the input
    if (expected.rejected.size() > 0)
    {
        if (std::none_of(expected.rejected.begin(), expected.rejected.end(), [&](const auto &lookFor) {
                std::string lcase = util::to_lower_copy(lookFor);

                auto result = lowerProgramOutput.find(lcase, 0);
                if (result == std::string::npos)
                {
                    return true;
                }

                log.push_back(AppRejection{lookFor, result});
                //                std::cout << std::format("found rejected string {} at {}\n", lookFor, result);
                return false;
            }))
        {
            // std::cout << "Failed due to finding rejected string.\n";
            return false;
        }
    }

    if (expected.test.mError != Tests::Errors::None)
    {
        // Search for the word ERROR
        auto result = lowerProgramOutput.find("error");
        if (result == std::string::npos)
        {
            log.push_back(AppMissingAnswer{"error"});
            return false;
        }
        log.push_back(AppFoundAnswer("error", result));
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
        std::string lcase = util::to_lower_copy(lookFor);

        auto result = lowerProgramOutput.find(lcase, 0);
        if (result == std::string::npos)
        {
            log.push_back(AppMissingAnswer(lookFor));
            // std::cout << std::format("Did not find expected result: {} \n", lookFor);
            return false;
        }
        // std::cout << std::format("Found result {} at {}\n", lookFor, result);
        // Verify answer (string must not be connected to a nother letter or number if it is then the answer is false)
        if (verify_whole_string(lowerProgramOutput, lookFor, result) == false)
        {
            log.push_back(AppMissingAnswer(lookFor));
            return false;
        }

        log.push_back(AppFoundAnswer(lookFor, result));
        return true;
    });
}

TestResult run_test(const Tests::Configuration::ExpectedResults &expected, std::string &app)
{

    TestResult result(expected.test);
    std::vector<std::string> cmdVector{app, "--load", expected.filename, "--guess"};

    for (auto &guess : expected.queries)
    {
        cmdVector.push_back(guess.as_base26_fmt());
    }
    if (expected.queries.size() == 0)
    {
        cmdVector.push_back("a0");
    }

    result.cmdline = std::accumulate(std::next(cmdVector.begin()), cmdVector.end(), cmdVector[0],
                                     [](std::string a, std::string &b) { return std::move(a) + ' ' + b; });

    std::cout << "Running test: " << result.cmdline << '\n';
    auto start = std::chrono::high_resolution_clock::now();
    auto ret = execute_app(cmdVector);
    auto end = std::chrono::high_resolution_clock::now();

    result.timeToRun = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    result.logs = std::move(ret.logs);
    result.passed = !ret.failed;
    result.programOutput = ret.appOutput;

    if (result.passed)
        result.passed = program_output_pass(expected, result.programOutput, result.logs);

    std::cout << std::format("Passed: [{}{}\u001b[0m] Took: [\u001b[33m{}\u001b[0m] \n",
                             result.passed == true ? "\u001b[32m" : "\u001b[31m", result.passed, result.timeToRun);
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

void print_report(const std::vector<TestResult> &tests)
{
    // Only print details on failed tests
    const char *red = "\u001b[31m";
    const char *green = "\u001b[32m";
    const char *yellow = "\u001b[33m";
    const char *blue = "\u001b[34m";
    const char *def = "\u001b[0m";

    for (const TestResult &tr : tests | std::views::filter([](const TestResult &t) { return !t.passed; }))
    {
        std::cout << "------------------------------------" << '\n';
        std::cout << "Test failed: " << yellow << tr.def.mName << def << '\n';
        std::cout << "Program Output: \n" << green;
        std::cout << tr.programOutput << def << '\n';
        std::cout << "Errors encountered: \n";

        int count = 1;

        for (const AppLogEntry &entry : tr.logs)
        {
            std::cout << def << count << ".  " << std::visit([](auto &t) -> std::string { return t; }, entry);
        }
    }
}

int main_run_tests(std::filesystem::path testPath, std::filesystem::path exe)
{

    Tests::Configuration loadMe;

    loadMe = Tests::toml_file_to_config(testPath);

    std::cout << "Finished Loading: " << testPath << '\n';

    auto report = run_all_tests(loadMe, exe);
    print_report(report);

    return 0;
}