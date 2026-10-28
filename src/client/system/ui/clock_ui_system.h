#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
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
  static inline ClockEnum type = ClockEnum::kMin;
  static inline uint64_t destroy;

  static inline SDL_FPoint pos;
  static bool Render();

  static void Open();
  static void Close();
};