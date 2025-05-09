#pragma once

#include "reproc++/reproc.hpp"

reproc::options default_process_options() {
  reproc::options options;
  reproc::stop_actions stop = {
      {reproc::stop::noop, reproc::milliseconds(0)},
      {reproc::stop::terminate, reproc::milliseconds(5000)},
      {reproc::stop::kill, reproc::milliseconds(2000)}};

  options.stop = stop;
  options.redirect.out.type = reproc::redirect::pipe;
  options.redirect.in.type = reproc::redirect::pipe;

  return options;
}
