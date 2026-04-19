#pragma once

#include "SDL3/SDL_rect.h"
class camera_game_instance {
public:
  static void load(float x, float y, float w, float h);
  static inline SDL_FRect camera;
};