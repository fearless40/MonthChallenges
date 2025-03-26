#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include "TestDefinition.hpp"
#include "stringutil.hpp"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <commandline.hpp>
#include <iostream>
#include <numeric>
#include <ranges>
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>
#include <thread>
#include <variant>

using namespace std::chrono_literals;

namespace Term
{
const char *red = "\u001b[31m";
const char *green = "\u001b[32m";
const char *yellow = "\u001b[33m";
const char *blue = "\u001b[34m";
const char *def = "\u001b[0m";
const char *bold = "\x1b[1m";

std::string make_color(std::size_t color)
{
    return std::format("\x1b[38;5;{}m", color);
}

}; // namespace Term

struct AppRejection
{
    std::string rejectionText;
    std::size_t location;
    std::size_t end;
    operator std::string() const noexcept
    {
        return as_string();
    }
    std::string as_string(const char *termColor = nullptr) const noexcept
    {
        return std::format("Found rejection text [{}{}{}] at location [{}]'\n", termColor != nullptr ? termColor : "",
                           rejectionText, Term::def, location);
    }
};

struct AppMissingAnswer
{
    std::string missingAnswer;
    operator std::string() const noexcept
    {
        return as_string();
    }
    std::string as_string(const char *termColor = nullptr) const noexcept
    {
        return std::format("Missing answer [{}] '\n", missingAnswer);
    }
};

struct AppFoundAnswer
{
    std::string answer;
    std::size_t location;
    std::size_t end;
    operator std::string() const noexcept
    {
        return as_string();
    }
    std::string as_string(const char *termColor = nullptr) const noexcept
    {
        return std::format("Found answer text [{}{}{}] at location [{}]'\n", termColor != nullptr ? termColor : "",
                           answer, Term::def, location);
    }
};

struct AppTimeOut
{
    std::chrono::milliseconds timeout;
    operator std::string() const noexcept
    {
        return as_string();
    }

    std::string as_string(const char *termColor = nullptr) const noexcept
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

    std::string as_string(const char *termColor = nullptr) const noexcept
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
    reproc::stop_actions stopActions{{reproc::stop::kill, 5000ms}, {reproc::stop::terminate, 10000ms}, {}};

    ExecuteResult exeResult;

    reproc::options options;
    options.stop = stopActions;

    const auto &ProgramOpt = CommandLine::get_program_options();

    options.deadline = reproc::milliseconds(ProgramOpt.runTimeOutMilliseconds);
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

        exeResult.logs.push_back(AppTimeOut{std::chrono::milliseconds(ProgramOpt.runTimeOutMilliseconds)});
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

    exeResult.failed = false;
    exeResult.appReturnCode = status;

    return exeResult;
}

struct SubString
{
    std::size_t start;
    std::size_t end;
};

std::optional<SubString> verify_whole_string(const std::string &output, const std::string &foundit,
                                             std::size_t startingPos)
{
    // looking for 3:  123 [bad]// 321 [bad]// 12,-3, [bad]// 1 2 3 [ok] // 1,2,3 [ok]//  (12)
    const std::string_view allowedSeperators{"=, []();|\n\r\t\0"};
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

    if (output.substr(startOfString, (endOfString - startOfString)) == foundit)
    {
        return SubString{startOfString, endOfString};
    }

    return {};
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
                    return true;

                auto check = verify_whole_string(appOutput, lcase, result);

                if (check)
                {
                    log.push_back(AppRejection{lookFor, check.value().start, check.value().end});
                    return false;
                }

                return true;
            }))
        {
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
        auto check = verify_whole_string(appOutput, "error", result);
        if (check)
        {
            log.push_back(AppFoundAnswer("error", check.value().start, check.value().end));
            return true;
        }
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
            return false;
        }
        // Verify answer (string must not be connected to a nother letter or number if it is then the answer is false)

        auto check = verify_whole_string(lowerProgramOutput, lookFor, result);
        if (!check)
        {
            log.push_back(AppMissingAnswer(lookFor));
            return false;
        }

        log.push_back(AppFoundAnswer(lookFor, check.value().start, check.value().end));
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

    for (const TestResult &tr : tests | std::views::filter([](const TestResult &t) { return !t.passed; }))
    {
        std::vector<std::pair<std::size_t, SubString>> highlites;

        for (std::size_t i = 0; i < tr.logs.size(); ++i)
        {
            auto result = std::visit(
                [&i](auto &&arg) -> std::pair<std::size_t, SubString> {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, AppRejection>)
                        return {i, {arg.location, arg.end}};
                    else if constexpr (std::is_same_v<T, AppFoundAnswer>)
                        return {i, {arg.location, arg.end}};
                    else
                        return {0, {0, 0}};
                },
                tr.logs[i]);

            if (result.second.start != 0 && result.second.end != 0)
                highlites.push_back(result);
        }

        std::ranges::sort(highlites, [](auto const &l, auto const &r) { return l.second.start < r.second.start; });

        std::cout << "------------------------------------" << '\n';
        std::cout << "Test failed: " << Term::yellow << tr.def.mName << Term::def << '\n';
        std::cout << "Program Output: \n" << Term::green;

        auto next = highlites.begin();
        for (std::size_t i = 0; i < tr.programOutput.size(); ++i)
        {
            if (next != highlites.end())
            {
                if ((*next).second.start == i)
                {
                    // std::cout << "--BOLD COLOR--\n";
                    std::cout << Term::bold << Term::make_color((std::distance(next, highlites.end()) + 1) * 2);
                }

                if ((*next).second.end == i)
                {
                    ++next;
                    // std::cout << "--END BOLD COLOR--\n";
                    std::cout << Term::def << Term::green;
                }
            }

            std::cout << tr.programOutput[i];
        }

        std::cout << Term::def;
        std::cout << "Errors encountered: \n";

        int count = 0;

        for (const AppLogEntry &entry : tr.logs)
        {
            std::cout << Term::def << ++count << ".  "
                      << std::visit(
                             [&count](auto &t) -> std::string {
                                 return t.as_string(Term::make_color((count + 1) * 2).c_str());
                             },
                             entry);
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