#pragma once

#include "SDL3/SDL_render.h"
#include <functional>
class FadeSystemInstance {
private:
  static inline int fade_in;
  static inline int mask_alpha;
  static inline std::function<void()> fn;
  static inline SDL_Texture *backgrnd;

public:
  static bool Run();
  static bool Render();

  static void EnterIn(std::function<void()> cb);
  static void EnterOut();
};