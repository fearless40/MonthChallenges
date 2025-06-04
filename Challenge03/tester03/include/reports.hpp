#pragma once
#include "virtualgames.hpp"
#include <ostream>

namespace report {
std::ostream &time(std::ostream &s, VirtualGames::TimeT const duration);
std::ostream &header();
}; // namespace report
