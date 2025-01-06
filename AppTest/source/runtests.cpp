#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include "TestDefinition.hpp"
#include "subprocess.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdio.h>

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

    std::vector<std::string> arguments{app, "--load", expected.filename, "--guess"};
    if (expected.queries.size() > 0)
    {

        arguments.push_back(guesses);
    }
    else
    {
        arguments.push_back("a0");
    }
    std::string arg2;
    for (const auto &st : arguments)
    {
        arg2 += st + " ";
    };

    std::cout << "Running the following test: " << arg2 << '\n';
    // auto result = sp::check_output(arguments);
    // auto result = sp::Popen(arguments, sp::output{sp::PIPE});
    // auto buf = result.communicate().first;
    // std::cout << buf.buf.data() << '\n';

    char psBuffer[128];
    FILE *pPipe;

    /* Run DIR so that it writes its output to a pipe. Open this
     * pipe with read text attribute so that we can read it
     * like a text file.
     */

    if ((pPipe = _popen(arg2.c_str(), "rt")) == NULL)
    {
        exit(1);
    }

    /* Read pipe until end of file, or an error occurs. */

    while (fgets(psBuffer, 128, pPipe))
    {
        std::cout << psBuffer;
    }

    int endOfFileVal = feof(pPipe);
    int closeReturnVal = _pclose(pPipe);

    if (endOfFileVal)
    {
        printf("\nProcess returned %d\n", closeReturnVal);
    }
    else
    {
        printf("Error: Failed to read the pipe to the end.\n");
    }
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