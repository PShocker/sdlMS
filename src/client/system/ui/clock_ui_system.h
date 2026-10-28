#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
namespace sdlms {

class ClockUiSystem {
private:
  static void RenderBackgrnd();
  static void RenderNumber(uint32_t num, float x, float y);
  static void RenderTime();

public:
  enum class ClockEnum {
    kMin,
    kHour,
  };
  static inline ClockEnum type_ = ClockEnum::kMin;
  static inline uint64_t destroy_;

  static inline SDL_FPoint pos_;
  static bool Render();

  static void Open();
  static void Close();
};

}  // namespace sdlms
