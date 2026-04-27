#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <flat_map>

class skill_ui_system {
private:
  static void render_backgrnd();
  static void render_tab();
  static void render_skill_entry();
  static void render_scroll();

  static SDL_FPoint load_wh();
  static uint8_t load_skill_num();
  
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;

  static inline uint8_t active_tab;
  static inline uint8_t sroll_num;

public:
  static inline SDL_FPoint pos;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};