#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_input.h"
#include <cstdint>
#include <optional>
class keybinding_ui_system {
private:
  static void render_button();

  static void render_backgrnd();
  static void render_backgrnd2();
  static void render_backgrnd_key();
  static void render_tab();
  static void render_remain_key();
  static SDL_FPoint load_wh();
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static void event_motion_bt_audio(SDL_Event *event);

  static void event_button_close();
  static void event_button_default();
  static void event_button_delete();
  static void event_button_quickSlot();
  static void event_button_prePage1();
  static void event_button_prePage2();
  static void event_button_prePage3();
  static void event_button_save();
  static void event_button_undo();

  static void event_backgrnd_key();
  static void event_remain_key();


  static bool event_button(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;
  static inline uint8_t active_tab;

public:
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static void render_remain_icon(int i, int x, int y, uint8_t alpha = 255);
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};