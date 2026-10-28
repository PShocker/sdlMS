#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_gauge.h"
#include <cstdint>
namespace sdlms {

class GaugeRenderSystem {
public:
  static bool RenderMob(SDL_FPoint pos, GameGauge &g);
  static bool RenderCharacter(SDL_FPoint pos, float hp_percent);
};

}  // namespace sdlms
