#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/ui/ui_base.h"
#include <array>
#include <cstdint>
#include <flat_map>
#include <string>

class worldmap_ui_system {
private:
  static void render_backgrnd();
  static void render_map();
  static void render_spot();

  static SDL_FPoint load_wh();
  static void event_top();
  static void event_drag(SDL_Event *event);

public:
  static inline SDL_FPoint pos;
  static inline std::u16string path;

  static inline std::array<uint32_t, 3> pos_ani_time;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};