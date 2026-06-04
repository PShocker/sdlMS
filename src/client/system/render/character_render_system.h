#pragma once

#include "src/client/game/game_character.h"
class character_render_system {
private:
  static bool render_afterimage(game_character &g_character);
  static void render_nametag(game_character &g_character);
  static void render_tomb(game_character &g_character);
  static bool render_effect_back(game_character &g_character);

public:
  static bool render(game_character &g_character);
  static bool render_character(game_character &g_character);
};