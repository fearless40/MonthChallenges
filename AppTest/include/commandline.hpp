#pragma once
#include <filesystem>
#include <optional> 

namespace CommandLine {

enum class RunMode {
    Generate,
    Run,
    Help,
    Interactive,
    Quit
};

struct Options {
    RunMode                 mode;
    std::filesystem::path   testFile;
    std::filesystem::path   testProgram;
    bool testNormalOnly {false};
    bool testAll {true};
    bool testHuge {false};
};




std::optional<Options> parse( int argc, const char * argv);

}