#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include "TestDefinition.hpp"
#include <iostream>
#include <chrono>


struct TestResult {
    std::chrono::duration timeToRun;
    Tests::Definition & def; 
    std::string programOutput; 
    bool passed; 
};

TestResult run_test( const Tests::Configuration::ExpectedResults & expected, std::filesystem::path app ) {

} 

std::vector<TestResult> run_all_tests( Tests::Configuration & config, std::filesystem::path app ){
    
    std::vector<TestResult> results;
    for( auto const & expected : config ){
        results.push_back(run_test(expected, app));
    }

}  

int main_run_tests( std::filesystem::path testPath, std::filesystem::path exe ){

    Tests::Configuration loadMe; 

    loadMe = Tests::toml_file_to_config( testPath );

    
    std::cout << "Finished Loading: " << testPath << '\n';
    


    return 0;
}