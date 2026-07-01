
#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <string>
class context_menu_ui_system {
private:
  static SDL_FPoint load_wh();
  static void render_backgrnd();
  static void render_button();

  static void event_button_info();
  static void event_button_save();
  static void event_button_whisper();
  static void event_button_party();
  static void event_button_trade();
  static void event_button_friend();
  static void event_button_chat();

  static bool event_button(SDL_Event *event);

public:
  static inline uint64_t client_id;
  static inline std::u16string client_name;

  static inline SDL_FPoint pos;
  static bool render();

  static bool cursor_in();

  static void open();
  static void close();
  static bool event(SDL_Event *event);
};