#pragma once

#include "SDL3/SDL_render.h"
#include <functional>
namespace sdlms {

class FadeSystemInstance {
private:
  static inline int fade_in_;
  static inline int mask_alpha_;
  static inline std::function<void()> fn_;
  static inline SDL_Texture *backgrnd_;

public:
  static bool Run();
  static bool Render();

  static void EnterIn(std::function<void()> cb);
  static void EnterOut();
};

}  // namespace sdlms
