#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <optional>
#include <string>
class server_mob {
public:
  uint32_t index;
  std::u16string id;
  int32_t rx0;
  int32_t rx1;

  SDL_FPoint pos;
  std::optional<uint64_t> hate_id;
  int64_t hp;
  int64_t mp;

  std::u16string action;

  float hspeed = 0;
  float vspeed = 0;

  float hforce = 0.0;
  float vforce = 0.0;

  bool visible = false;

  int64_t fh;
};