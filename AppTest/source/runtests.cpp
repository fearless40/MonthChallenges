#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include "TestDefinition.hpp"
#include "subprocess.hpp"
#include <chrono>
#include <iostream>

struct TestResult
{
    std::chrono::milliseconds timeToRun;
    Tests::Definition &def;
    std::string programOutput;
    bool passed;
};

void run_test(const Tests::Configuration::ExpectedResults &expected, std::string &app)
{
    namespace sp = subprocess;

    std::string guesses;
    guesses.reserve(512);

    for (auto &guess : expected.queries)
    {
        guesses.append(guess.as_base26_fmt());
        guesses.append(" ");
    }

    std::vector<std::string> arguments{app, "--load", expected.filename};
    if (expected.queries.size() > 0)
    {
        arguments.emplace_back("--guess");
        arguments.push_back(guesses);
    }
    std::cout << "Running the following test: " << app << " --load " << expected.filename << " --guess " << guesses
              << '\n';
    auto result = sp::check_output(arguments);
    std::cout << result.buf.data() << '\n';
}

std::vector<TestResult> run_all_tests(Tests::Configuration &config, std::filesystem::path app)
{

    std::vector<TestResult> results;
    std::string appString = app.generic_string();
    for (auto const &expected : config)
    {
        run_test(expected, appString);
        // results.push_back(run_test(expected, app));
    }

    return {};
}

int main_run_tests(std::filesystem::path testPath, std::filesystem::path exe)
{

    Tests::Configuration loadMe;

    loadMe = Tests::toml_file_to_config(testPath);

    std::cout << "Finished Loading: " << testPath << '\n';

    run_all_tests(loadMe, exe);

    return 0;
}