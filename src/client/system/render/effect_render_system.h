#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
class effect_render_system {
private:
  static void render_damage(SDL_FPoint pos, game_effect &g_effect);
  static bool render_custom(SDL_FPoint pos, game_effect &g_effect, bool flip);
  static void render_ski_affected(SDL_FPoint pos, game_effect &g_effect,
                                  bool flip);
  static void render_ski_hit(SDL_FPoint pos, game_effect &g_effect, bool flip);
  static void render_ski_region(game_effect &g_effect);

public:
  static void render_effect(SDL_FPoint pos, game_effect &g_effect);
  static void render_ski_use(SDL_FPoint pos, game_effect &g_effect, bool flip);

  static bool render(SDL_FPoint pos, game_effect &g_effect, bool flip);
  static bool render(game_effect &g_effect);
  static bool render_mob_effects(game_mob &g_mob, bool front);
  static bool render_character_effects(game_character *g_character, bool front);
};