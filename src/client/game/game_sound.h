#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class game_sound {
public:
  uint32_t offset = 0; // 记录当前播放的位置
  uint32_t delay = 0;  // 记录当前播放是否延迟,数值表示延迟时间
  std::u16string path;
};