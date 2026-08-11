#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>

class skill_ui_system {
private:
  static std::optional<std::u16string> load_mouse_ski();
  static void render_backgrnd();
  static void render_tab();
  static void render_skill_entry();
  static void render_scroll();
  static bool render_info();
  static void render_button();
  static void render_book();
  static void render_point();

  static SDL_FPoint load_wh();
  static uint8_t load_skill_num();

  static bool event_click_ski(SDL_Event *event);
  static bool event_click_tab(SDL_Event *event);
  static void event_click_vscr(SDL_Event *event);

  static void event_motion(SDL_Event *event);
  static void event_button_ski_up();
  static bool event_button(SDL_Event *event);
  static void event_close();
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;

  static inline uint8_t active_tab;
  static inline uint8_t page;

public:
  static inline SDL_FPoint pos;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};