#pragma once

#include "SDL3/SDL_events.h"
class chat_log_ui_system {
private:
  static void render_chats();
  static void render_vscr();

  static void event_click_chat_vscr();
  static bool event_button(SDL_Event *event);

  static SDL_FPoint load_wh();

public:
  static inline int page;
  static inline SDL_FPoint pos;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();
};