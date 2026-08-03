#pragma once

#include "SDL3/SDL_rect.h"
#include <cstdint>
class clock_ui_system {
private:
  static void render_backgrnd();
  static void render_number(uint32_t num, float x, float y);
  static void render_time();

public:
  enum class clock_enum {
    min,
    hour,
  };
  static inline clock_enum type = clock_enum::min;
  static inline uint64_t destroy;

  static inline SDL_FPoint pos;
  static bool render();

  static void open();
  static void close();
};