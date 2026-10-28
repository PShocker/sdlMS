#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>

class GameInput {
public:
  std::string type;
  std::string val;
  std::string sub_val;

  auto operator<=>(const GameInput &) const = default;
};