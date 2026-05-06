#pragma once

#include "src/client/game/game_character.h"
#include "src/client/game/game_drop.h"
class drop_logic_system {
public:
  static void pick(game_character &g_character);
  static void pick(game_pet &g_pet);
  static bool run();
};