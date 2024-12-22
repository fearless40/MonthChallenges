

#include <iostream>
#include "clipp.h"
#include "commandline.hpp"



namespace CommandLine {

std::optional<std::filesystem::path> ensure_file_exists( std::string_view filename ){
    if( std::filesystem::exists( filename ) ) {
        return filename;
    }; 
    return {};
}

bool does_file_exists(std::string_view filename ) {
    if( ensure_file_exists( filename )) 
        return true;
    return false;
}



std::optional<Options> parse( int argc, char * argv[]) { 
    using namespace clipp; 


    Options opt; 
    
    RunMode run {RunMode::Interactive}; 
    std::string testFile;
    std::string testProgram;

    bool testNormalOnly {false};
    bool testAll {true};
    bool testHuge {false};


    auto commandRun = ( 
        clipp::command("run").set(run, RunMode::Run),  
        value("test file", testFile) % "A test file generated by the generate command.",
        required("-p", "--program").set(testProgram) % "Specifify the program to test. It should follow the Challenge descriptions."
    );

    auto commandGenerate = (
        clipp::command("generate").set(run, RunMode::Generate),
        value("test file", testFile) % "The test file that desribes the tests and location of the test files.",
        ( option("--validOnly").set(testNormalOnly) % "Generate only test files that are valid. No errors in the test files." | 
          option("--all").set(testAll) % "Generate all test files this is the default."
        ),
        option("--huge").set(testHuge) % "Generate a huge test file."
    );

    auto cli = ( 
        commandRun | commandGenerate | command("help").set(run,RunMode::Help),
        option("-v", "--version").call([]{std::cout << "version 1.0\n\n";}).doc("show version")  
    );


    auto result = clipp::parse( argc, argv, cli ); 

    if( !result ){
        std::cout << clipp::usage_lines(cli, "apptest") << '/n'; 
        run = RunMode::Quit;
    }

    if( run == RunMode::Help ){
        std::cout << "Usage: /n";
        std::cout << clipp::make_man_page(cli, "apptest") << '/n';
        run = RunMode::Quit;
    }

    opt.mode = run;

    // Validate the files before continuing. 

    switch (run)
    {
    case RunMode::Run:
        {
            auto fileProgram = ensure_file_exists(testProgram);
            if( !fileProgram ) {
                std::cout << "The test program file is not found: " << testProgram << '/n';
                return {};
            }
            
            opt.testProgram = fileProgram.value(); 

            auto fileTest = ensure_file_exists(testFile);
            if( !fileTest ){
                std::cout << "The test file is not found: " << testFile << '/n';
                return {};
            }

            opt.mode = run;
            opt.testFile = fileTest.value();
            return opt;
        }
        break;

    case RunMode::Generate: 
    
        if( does_file_exists( testFile ))
        {
            std::cout << "Found an existing test file at: " << testFile << '/n';
            std::cout << "Generate a new file (y/n):";
            bool overwrite {false};
            std::cin >> overwrite;
            if( !overwrite ) {
                std::cout << "/nAborting generating the test./n"; 
                return {};
            }

            if( (testNormalOnly == true) && (testAll == true)) {
                std::cout << "Cannot run error tests and only normal test options at the same time. --validOnly and --all are mutually exclusive./n";
                return {};
            }

            opt.testFile = testFile;
            opt.testHuge = testHuge;
            opt.testAll = testAll;
            opt.testNormalOnly = testNormalOnly;

            return opt;
        }
        break;

    case RunMode::Help:
        return {};

    case RunMode::Quit:
        return {};

    case RunMode::Interactive:
        opt.mode = run;
        return opt; 

   }
    



}
}