#pragma once

#include "SDL3/SDL_rect.h"
namespace sdlms {

class CameraGameInstance {
public:
  static void Reset();
  static void Load(float x, float y, float w, float h);
  static inline SDL_FRect camera_;
};

}  // namespace sdlms
