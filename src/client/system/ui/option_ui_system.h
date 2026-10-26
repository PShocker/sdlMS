#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <optional>
class option_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_tab();
  static void render_option();

  static bool event_button(SDL_Event *event);
  static void event_button_ok();

  static void event_tab(SDL_Event *event);

  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);
  static void event_motion_bt_audio(SDL_Event *event);

  static SDL_FPoint load_wh();

  static inline std::optional<SDL_FPoint> drag;

public:
  static inline uint8_t hp_warn;
  static inline uint8_t mp_warn;
  enum class enemy_info_enum {
    name_hp,
    name,
    hp,
  };
  static inline enemy_info_enum enemy_info;

  static inline uint8_t active_tab;

  static inline SDL_FPoint pos;
  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
  static void toggle();
};