#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_character.h"
class character_create_ui_system {
private:
  static bool back_animate();

  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();
  static void render_effect();
  static void render_banner();
  static void render_stat();
  static void render_character();
  static void render_custom();

  static void event_button_select();
  static void event_button_new();
  static void event_button_delete();
  static void event_button_back();

  static bool event_button(SDL_Event *event);

  static inline int str_point;
  static inline int dex_point;
  static inline int int_point;
  static inline int luk_point;

public:
  static bool render();
  static bool event(SDL_Event *event);

  static inline game_character g_character;
};