#pragma once

#include "SDL3/SDL_rect.h"
#include "ui_base.h"
#include <cstdint>
#include <string>
class ui_minimap : public ui_base {
public:
  uint32_t map_id;
  std::u16string map_name;
  std::u16string map_street;
};