#include "runner.hpp"
#include "reproc++/reproc.hpp"
#include <iostream>
#include <system_error>

void stripn(unsigned char *buff, std::size_t size) {
  for (std::size_t i = 0; i < size; ++i) {
    if (buff[i] != '\n')
      std::cout << buff[i];
  }
  std::cout << ',';
}

bool test(ProgramOptions::Options const &opt) {
  reproc::process subapp;
  reproc::options o;
  reproc::stop_actions stop = {
      {reproc::stop::noop, reproc::milliseconds(0)},
      {reproc::stop::terminate, reproc::milliseconds(5000)},
      {reproc::stop::kill, reproc::milliseconds(2000)}};

  o.stop = stop;
  o.redirect.out.type = reproc::redirect::pipe;
  o.redirect.in.type = reproc::redirect::pipe;

  std::array<std::string, 4> cmdline{opt.program_to_test, "run", "--ai", "0"};
  auto ec = subapp.start(cmdline, o);

  if (ec == std::errc::no_such_file_or_directory) {
    std::cout << "Program not found.\n";
    return false;
  }

  unsigned char buffer[256];

  for (int i = 0; i < 5; ++i) {
    std::size_t sz;
    std::tie(sz, ec) =
        subapp.read(reproc::stream::out, (unsigned char *)&buffer, 255);
    stripn(buffer, sz);

    subapp.write((unsigned char *)&"M\n", 2);
  }

  subapp.write((unsigned char *)&"Q\n", 2);

  std::size_t sz;
  std::tie(sz, ec) =
      subapp.read(reproc::stream::out, (unsigned char *)&buffer, 255);
  stripn(buffer, sz);

  o.stop.first = {reproc::stop::wait, reproc::milliseconds(10000)};

  int status = 0;
  std::tie(status, ec) = subapp.stop(o.stop);

  if (ec) {
    std::cout << "Error: " << ec.message() << '\n';
  }

  return true;
}
