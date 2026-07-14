#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
#include <vector>
class character_stat_ui_system {
private:
  static void render_backgrnd();
  static void render_text();
  static void render_remain_ap();
  static void render_button();
  static SDL_FPoint load_wh();

  static void event_button_hp_inc();
  static void event_button_hp_inc_max();
  static void event_button_mp_inc();
  static void event_button_mp_inc_max();
  static void event_button_str_inc();
  static void event_button_str_inc_max();
  static void event_button_dex_inc();
  static void event_button_dex_inc_max();
  static void event_button_int_inc();
  static void event_button_int_inc_max();
  static void event_button_luk_inc();
  static void event_button_luk_inc_max();

  static void event_button_detail_show();
  static void event_button_detail_hide();
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static bool event_button(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;

  static inline bool detail;

public:
  static inline SDL_FPoint pos;
  static bool render();

  static bool cursor_in();

  static void open();
  static void close();
  static bool event(SDL_Event *event);

  static void toggle();
};