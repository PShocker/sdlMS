#pragma once

#include "src/client/game/game_mob.h"
class MobRenderSystem {
private:
  static void render_nametag(const std::u16string &text, SDL_FPoint p);
  static bool render_mob(GameMob &g_mob);
  static bool render_gauge(GameMob &g_mob);
  static bool render_nametag(GameMob &g_mob);
  static bool render_effect_back(GameMob &g_mob);
  static void render_debuff(GameMob &g_mob);

public:
  static bool render(GameMob &g_mob);
};