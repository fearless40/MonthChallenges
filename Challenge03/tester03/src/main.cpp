#include <iostream>

#include "commandline.hpp"
#include "programoptions.hpp"
#include "runner.hpp"

int main(int argc, char *argv[]) {

  auto opt = commandline::parse(argc, argv);

  switch (opt.mode) {
  case ProgramOptions::RunMode::version:
    std::cout << "Tester03 version 1.0\n";
    return 0;
  case ProgramOptions::RunMode::help:
  case ProgramOptions::RunMode::error:
    return 0;
  case ProgramOptions::RunMode::test:
    test(opt);
  }
}
