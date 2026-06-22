#pragma once

#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"
#include <cstdint>
#include <flat_map>
class keyboard_game_instance {
private:
  static void reset();

public:
  static void load();
  static inline std::flat_map<SDL_Scancode, game_input> data;
};