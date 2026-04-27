#pragma once

#include "SDL3/SDL_events.h"
#include <cstdint>
#include <flat_map>

class statusbar_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  static void render_quickSlot();
  static SDL_FPoint load_wh();

  static bool event_button(SDL_Event *event);
  // button_func
  static void event_button_cashshop();
  static void event_button_menu();
  static void event_button_shortcut();
  static void event_button_mailbox();
  static void event_button_equip();
  static void event_button_inven();
  static void event_button_stat();
  static void event_button_skill();
  static void event_button_keybind();
  static void event_button_quickslot();
  static void event_button_chatlog();

  static inline bool quickSlot=true;

public:
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
};