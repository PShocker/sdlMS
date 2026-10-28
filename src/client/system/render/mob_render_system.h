#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_MOB_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_MOB_RENDER_SYSTEM_H_

#include "src/client/game/game_mob.h"
namespace sdlms {

class MobRenderSystem {
 private:
  static void RenderNametag(const std::u16string& text, SDL_FPoint p);
  static bool RenderMob(GameMob& g_mob);
  static bool RenderGauge(GameMob& g_mob);
  static bool RenderNametag(GameMob& g_mob);
  static bool RenderEffectBack(GameMob& g_mob);
  static void RenderDebuff(GameMob& g_mob);

 public:
  static bool Render(GameMob& g_mob);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_MOB_RENDER_SYSTEM_H_
