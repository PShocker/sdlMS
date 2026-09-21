#pragma once

#include "SDL3/SDL_rect.h"
#include <any>
#include <cstdint>
#include <string>

class server_region_skill {
public:
  uint64_t client_id;
  uint64_t map_id;
  std::u16string id;
  uint8_t lv;

  SDL_FPoint pos;
  uint8_t page;
  bool flip;

  uint64_t start;
  uint64_t end;
  int64_t val;

  std::any data;
};