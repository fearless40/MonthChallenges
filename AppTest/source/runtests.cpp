#include "runtests.hpp"
#include "TestConfigTOML.hpp"
#include <iostream>


int main_run_tests( std::filesystem::path testPath, std::filesystem::path exe ){

    Tests::Configuration loadMe; 

    loadMe = Tests::toml_file_to_config( testPath );

    std::filesystem::path newFile = testPath;

    newFile.replace_filename("rewritten.txt"); 

    std::cout << "Finished Loading: " << testPath << '\n';
    std::cout << "Regenerating the config file from loaded file: " << newFile << '\n';

    Tests::config_to_toml_file( loadMe, newFile);

    std::cout << "Finished\n";

    return 0;
}