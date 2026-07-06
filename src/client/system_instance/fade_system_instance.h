#pragma once

#include "SDL3/SDL_render.h"
#include <functional>
class fade_system_instance {
private:
  static inline int fade_in;
  static inline int mask_alpha;
  static inline std::function<void()> fn;
  static inline SDL_Texture *backgrnd;

public:
  static bool run();
  static bool render();

  static void enter_in(std::function<void()> cb);
  static void enter_out();
};