#pragma once

#include "src/client/game/game_character.h"
#include <cstdint>
class scene_system_instance {
public:
  static inline uint32_t map_id;
  static void enter(uint32_t map_id);
  static void enter_prepare(uint32_t map_id);
  static void exit_prepare();

private:
  static bool render_game();
  static uint8_t load_character_layer(game_character &g_character);
};