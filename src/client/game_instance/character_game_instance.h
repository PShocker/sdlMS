#pragma once

#include "src/client/game/game_character.h"
#include <cstdint>
#include <flat_map>
#include <vector>
class character_game_instance {
public:
  static void load_character();
  static void add_character(uint32_t map_id);

  static inline game_character self;
  static inline uint64_t invincible;
  static inline SDL_FRect self_r{-20, -50, 30, 45};

  static inline std::flat_map<uint64_t, game_character> data;
};