#pragma once

#include <cstdint>
#include <ctime>
#include <random>

class random_game_instance {
public:
  static inline std::mt19937 gen{std::random_device{}()};
};