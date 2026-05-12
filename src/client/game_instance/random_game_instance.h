#pragma once

#include <cstdint>
#include <ctime>
#include <random>

class random_game_instance {
public:
  static inline uint32_t seed = std::time(0);
  static inline std::mt19937 gen;
};