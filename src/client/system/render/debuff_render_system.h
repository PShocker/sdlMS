#pragma once

#include "SDL3/SDL_rect.h"
#include "src/client/game/game_character.h"
#include "src/client/game/game_debuff.h"
#include "src/client/game/game_mob.h"
class DebuffRenderSystem {
private:
  static void Render(GameDebuff::DebuffType type, SDL_FPoint pos);

public:
  static void RenderCharacter(GameCharacter &character);
  static void RenderMob(GameMob &mob);
};