#pragma once

#include <chrono>

using AIID = std::size_t;

enum class AIStats_FinishingState {
  WonGame,
  TooManyGuess,
  ErroredOut,
  Unknown
};

struct AIStats_SingleRun {
  std::chrono::milliseconds shortest_answer{0};
  std::chrono::milliseconds longest_answer{0};
  std::chrono::milliseconds avg_answer{0};
  std::chrono::milliseconds total_time{0};

  std::size_t repeat_guess_count{0};
  std::size_t invalid_guess_count{0};

  AIStats_FinishingState state{AIStats_FinishingState::Unknown};
  AIID aiid;
};

struct AIRun {
  // Game board
  // Guess list
  // Stats
  //
};
