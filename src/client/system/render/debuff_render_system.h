#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_mob.h"
class debuff_render_system {
private:
  static void render(game_debuff::debuff_type type, SDL_FPoint pos);

public:
  static void render_character(game_character &character);
  static void render_mob(game_mob &mob);
};