#ifndef SDLMS_SRC_CLIENT_SYSTEM_RENDER_EFFECT_RENDER_SYSTEM_H_
#define SDLMS_SRC_CLIENT_SYSTEM_RENDER_EFFECT_RENDER_SYSTEM_H_

#include <SDL3/SDL_rect.h>
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
namespace sdlms {

class EffectRenderSystem {
private:
  static void RenderDamage(SDL_FPoint pos, GameEffect &g_effect);
  static bool RenderCustom(SDL_FPoint pos, GameEffect &g_effect, bool flip);
  static void RenderSkiAffected(SDL_FPoint pos, GameEffect &g_effect,
                                  bool flip);
  static void RenderSkiHit(SDL_FPoint pos, GameEffect &g_effect, bool flip);
  static void RenderSkiRegion(GameEffect &g_effect);

public:
  static void RenderEffect(SDL_FPoint pos, GameEffect &g_effect);
  static void RenderSkiUse(SDL_FPoint pos, GameEffect &g_effect, bool flip);

  static bool Render(SDL_FPoint pos, GameEffect &g_effect, bool flip);
  static bool Render(GameEffect &g_effect);
  static bool RenderMobEffects(GameMob &g_mob, bool front);
  static bool RenderCharacterEffects(GameCharacter *g_character, bool front);
};

}  // namespace sdlms

#endif  // SDLMS_SRC_CLIENT_SYSTEM_RENDER_EFFECT_RENDER_SYSTEM_H_
