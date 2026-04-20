#pragma once

#include "SDL3/SDL_rect.h"
#include "game_chatballoon.h"
#include <cstdint>
#include <optional>
#include <string>
class game_npc {
public:
  std::u16string id;
  int32_t rx0;
  int32_t rx1;
  int32_t fh;

  SDL_FPoint pos;

  std::u16string action;

  uint8_t alpha = 255;

  uint32_t ani_index = 0;
  uint32_t ani_time = 0;

  std::optional<game_chatballoon> chatballoon;
};