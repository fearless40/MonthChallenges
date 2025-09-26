#include "commandline.hpp"
#include "clipp.h"
#include "programoptions.hpp"
#include <iostream>

namespace commandline {

ProgramOptions::Options parse(int argc, char *argv[]) {

  using namespace clipp;

  ProgramOptions::Options opt;

  auto match_report_type = [](const std::string &arg) {
    return arg == "csv" || arg == "report";
  };

  auto run_cli =
      (clipp::command("run").set(opt.mode, ProgramOptions::RunMode::test),
       (option("--iterations") & value("iterations", opt.nbrIterations) %
                                     "Number of games to test against the AI."),
       (option("--rows") &
        value("rows", opt.rowSize) % "Number of rows. Default is 10."),
       (option("--cols") &
        value("cols", opt.colSize) % "Number of cols. Default is 10."),
       (option("--ships") &
        value("smallest ship", opt.smallestShip) %
            "Smallest ship default is 2" &
        value("largest ship", opt.largestShip) % "Largest ship default is 5"),
       (option("-o", "--output") &
        value("report file", opt.result_file) %
            "Write results to a file. Default is results.txt"),
       (option("--fileformat") &
        (value(match_report_type, "csv or filter")
             .call([&](std::string const &value) {
               if (value == "csv")
                 opt.filemode = ProgramOptions::FileOutput::csv;
               else
                 opt.filemode = ProgramOptions::FileOutput::report;
             }))),
       (option("--layout") &
        value("layout file", opt.ship_layout_file) %
            "Load a layout file to test non random ship placements"),
       (option("--wait") &
        value("wait time", opt.wait_upto_millis) %
            "Amount of time in killingseconds to wait for an answer before "
            "killing the testing program. Default is 500 ms. "),
       repeatable((option("--ai") & value("ai id", opt.ai_id_to_test))),
       (value("program", opt.program_to_test) %
        "Executable program to test that follows Challenge03 protocol."));

  auto version_cli = (clipp::command("version").set(
      opt.mode, ProgramOptions::RunMode::version));
  auto help_cli =
      (clipp::command("help").set(opt.mode, ProgramOptions::RunMode::help));

  auto cli = run_cli | version_cli | help_cli;

  if (!parse(argc, argv, cli)) {
    std::cout << clipp::usage_lines(cli, "tester03") << '\n';
    opt.mode = ProgramOptions::RunMode::error;
    return opt;
  }

  if (opt.mode == ProgramOptions::RunMode::help) {
    std::cout << clipp::make_man_page(cli, "tester03") << '\n';
    return opt;
  }

  if (opt.ai_id_to_test.size() > 0)
    opt.all_ai = false;
  else
    opt.all_ai = true;

  if (opt.program_to_test == "") {
    std::cout << clipp::usage_lines(cli, "tester03") << '\n'
              << "Missing program to test.\n";
    opt.mode = ProgramOptions::RunMode::error;
  }

  return opt;
}
} // namespace commandline
