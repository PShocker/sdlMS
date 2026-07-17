#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <optional>
#include <string>

class game_ball {
public:
  std::u16string itm_id;
  std::u16string ski_id;

  SDL_FPoint goal;
  std::optional<uint32_t> mob_index;

  uint64_t delay;
  uint64_t destory;
  int32_t rotate = 0;

  bool pierce = false; // 穿透
  SDL_FPoint pos;
};