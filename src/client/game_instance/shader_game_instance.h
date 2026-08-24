#pragma once

#include "SDL3/SDL_render.h"
#include <flat_map>
#include <vector>
enum class shader_enum {
  grayscale,
  outline,
};

class shader_game_instance {
public:
  static bool init();
  static void start(shader_enum type);
  static void end();
};