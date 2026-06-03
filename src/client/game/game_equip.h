#pragma once

#include "SDL3/SDL_rect.h"
#include "game_scroll.h"
#include <cstdint>
#include <string>
#include <vector>
class game_equip {
public:
  std::u16string id;

  std::vector<game_scroll> scroll;
};