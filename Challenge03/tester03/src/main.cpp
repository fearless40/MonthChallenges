#include <iostream>

#include "commandline.hpp"
#include "programoptions.hpp"
#include "runner.hpp"
#include "showreport.hpp"

int main(int argc, char *argv[]) {

  auto opt = commandline::parse(argc, argv);

  ui::start(opt);
  return 0;

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
