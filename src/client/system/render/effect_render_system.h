#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_effect.h"
#include "src/client/game/game_mob.h"
class effect_render_system {
private:
  static void render_effect(SDL_FPoint pos, game_effect &g_effect);
  static void render_damage(SDL_FPoint pos, game_effect &g_effect);
  static bool render_custom(SDL_FPoint pos, game_effect &g_effect, bool flip);
  static void render_ski_use(SDL_FPoint pos, game_effect &g_effect, bool flip);
  static void render_ski_hit(SDL_FPoint pos, game_effect &g_effect, bool flip);

public:
  static bool render(SDL_FPoint pos, game_effect &g_effect, bool flip);
  static bool render(game_effect &g_effect);
  static bool render_mob_back(game_mob &g_mob);
  static bool render_character_back(game_character *g_character);
};