#pragma once

#include <cstdint>
#include <ctime>
#include <random>

class RandomGameInstance {
public:
  static inline std::mt19937 gen{std::random_device{}()};
};