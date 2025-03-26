#pragma once
#include <random>

namespace Random {

inline static std::random_device randomDevice;
inline static std::mt19937 randomGen(randomDevice());

/*static bool coin_flip()
{
    std::uniform_int_distribution<> r(0, 1);
    return r(randomGen) == 0 ? false : true;
}*/

template <typename T> auto between(T min, T max) -> T {
  std::uniform_int_distribution<T> r(min, max);
  return r(randomGen);
}

} // namespace Random
