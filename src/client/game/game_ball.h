#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <optional>
#include <string>

class game_ball {
public:
  std::u16string path;

  SDL_FPoint goal;
  std::optional<uint32_t> mob_index;

  uint64_t delay;
  int speed;

  SDL_FPoint pos;
  bool flip;

  uint32_t ani_index = 0;
  uint32_t ani_time = 0;
};