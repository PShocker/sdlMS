#pragma once

#include "SDL3/SDL_render.h"
#include <functional>
class fade_system_instance {
private:
  static bool run();
  static bool render();

  static inline int mask_alpha;
  static inline std::function<void()> fn;
  static inline SDL_Texture *backgrnd;

public:
  static void enter(std::function<void()> cb);
};