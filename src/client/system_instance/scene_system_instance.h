#pragma once

#include "src/client/game/game_character.h"
#include <cstdint>
#include <string>
class scene_system_instance {
public:
  static inline uint32_t map_id;
  static inline uint32_t prepare_map_id;
  static void enter(uint32_t map_id);
  static void enter_prepare(uint32_t map_id, const std::u16string &pn,
                            uint8_t i);
private:
  static bool render_game();
};