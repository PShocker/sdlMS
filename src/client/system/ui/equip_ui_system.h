#pragma once

#include "SDL3/SDL_events.h"
class equip_ui_system {
public:
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);
};