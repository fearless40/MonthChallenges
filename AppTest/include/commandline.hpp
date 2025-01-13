#pragma once
#include <filesystem>
#include <optional>

namespace CommandLine
{

enum class RunMode
{
    Generate,
    Run,
    Help,
    Interactive,
    Quit
};

enum class TestModes
{
    All,
    NoErrors,
};

struct Options
{
    RunMode mode{RunMode::Quit};
    std::filesystem::path testFile{};
    std::filesystem::path testProgram{};
    TestModes tests{TestModes::All};
    bool huge{false};
    bool overwrite{false};
    int runTimeOutMilliseconds{500};
};

std::optional<Options> parse(int argc, char *argv[]);

const Options &get_program_options();

} // namespace CommandLine