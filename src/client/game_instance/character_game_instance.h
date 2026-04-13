#pragma once

#include "src/client/game/game_character.h"
#include <vector>
class character_game_instance {
public:
  static void load_character();
  static void add_character(uint32_t map_id);

  static inline game_character self;
  static inline std::vector<game_character> data;
};