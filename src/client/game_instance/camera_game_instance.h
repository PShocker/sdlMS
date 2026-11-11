#pragma once

#include <SDL3/SDL_rect.h>

#include <cstdint>

namespace sdlms {

class CameraGameInstance {
 public:
  static void Reset();
  static void Load(float x, float y, float w, float h);
  static inline SDL_FRect camera_;
  // v95 Effect_Tremble：命中震屏（强度 / 持续毫秒）
  static void Shake(float power, uint64_t ms);
  static inline float shake_power_ = 0.0f;
  static inline uint64_t shake_end_ = 0;
};

}  // namespace sdlms
