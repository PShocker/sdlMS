#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
#include <vector>

class game_input {
public:
  std::string type;
  std::string val;
  std::string val2;

  auto operator<=>(const game_input &) const = default;
};