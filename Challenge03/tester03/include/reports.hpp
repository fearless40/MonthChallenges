#pragma once
#include "virtualgames.hpp"
#include <ostream>

namespace report {
void print_game_board(std::ostream &s, battleship::GameLayout const &layout,
                      battleship::Ships const &ships);

void print_single_game_stats(std::ostream &s, std::size_t id,
                             const VirtualGames::Game &game);
void print_colors_on();
void print_colors_off();

void print_all_moves(std::ostream &s, const VirtualGames &games);
void print_global_stats(std::ostream &s, const VirtualGames &games);

}; // namespace report
