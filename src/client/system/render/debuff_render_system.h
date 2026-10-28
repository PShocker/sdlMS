#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_mob.h"
class DebuffRenderSystem {
private:
  static void render(GameDebuff::DebuffType type, SDL_FPoint pos);

public:
  static void render_character(GameCharacter &character);
  static void render_mob(GameMob &mob);
};