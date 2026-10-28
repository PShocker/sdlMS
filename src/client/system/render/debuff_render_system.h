#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_DEBUFF_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_DEBUFF_RENDER_SYSTEM_H_

#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_mob.h"

#include <SDL3/SDL_rect.h>

namespace sdlms {

class DebuffRenderSystem {
 private:
  static void Render(GameDebuff::DebuffType type, SDL_FPoint pos);

 public:
  static void RenderCharacter(GameCharacter& character);
  static void RenderMob(GameMob& mob);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_DEBUFF_RENDER_SYSTEM_H_
