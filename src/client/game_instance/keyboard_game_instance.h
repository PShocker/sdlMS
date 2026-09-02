#pragma once

#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"
#include "src/client/game/game_save.h"
#include <cstdint>
#include <flat_map>
class keyboard_game_instance {
private:
  static void reset();

public:
  static void load(const character_save &cs);
  static void load();
  static inline std::flat_map<SDL_Scancode, game_input> data;
  static SDL_Scancode find_key_by_val(const std::string &target_val);
};