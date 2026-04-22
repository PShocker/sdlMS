#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/ui/ui_base.h"
#include <cstdint>
#include <flat_map>
#include <string>

class worldmap_ui_system {
private:
  static void render_backgrnd();

public:
  static inline SDL_FPoint pos;
  static inline std::u16string path;

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};