#pragma once

#include "SDL3/SDL_rect.h"
#include "ui_base.h"
#include <cstdint>
class ui_statusbar_hp : public ui_base {
public:
  uint32_t prev_hp;
  float prev_percent;
  bool alert;
};