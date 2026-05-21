#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <optional>

class game_ball {
private:
  std::optional<SDL_FPoint> target_point = std::nullopt;
  // 摧毁时间
  uint64_t destory;
  int32_t rotate = 0;
  
  SDL_FPoint pos;
};