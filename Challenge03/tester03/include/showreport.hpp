#pragma once
#include "programoptions.hpp"
#include "virtualgames.hpp"
#include <vector>
namespace ui {
void start(ProgramOptions::Options const &opt,
           std::vector<VirtualGames> &games);
}
