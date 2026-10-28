#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_GAUGE_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_GAUGE_RENDER_SYSTEM_H_

#include "src/client/game/game_gauge.h"

#include <SDL3/SDL_rect.h>

#include <cstdint>
namespace sdlms {

class GaugeRenderSystem {
 public:
  static bool RenderMob(SDL_FPoint pos, GameGauge& g);
  static bool RenderCharacter(SDL_FPoint pos, float hp_percent);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_GAUGE_RENDER_SYSTEM_H_
