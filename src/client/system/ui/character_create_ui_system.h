#pragma once

#include "SDL3/SDL_events.h"
#include "src/client/game/game_character.h"
#include <string>
#include <vector>
class character_create_ui_system {
private:
  static std::vector<std::u16string> load_default_face();
  static std::vector<std::u16string> load_default_hair();
  static std::vector<std::u16string> load_default_top();
  static std::vector<std::u16string> load_default_bottom();
  static std::vector<std::u16string> load_default_shoes();
  static std::vector<std::u16string> load_default_weapon();

  static std::vector<std::u16string> load_haircolors();
  static bool back_animate();

  static SDL_FPoint load_pos();
  static void render_backgrnd();
  static void render_button();
  static void render_effect();
  static void render_banner();
  static void render_stat();
  static void render_character();
  static void render_custom();

  static void event_button_gender_prev();
  static void event_button_gender_next();

  static void event_button_face_prev();
  static void event_button_face_next();

  static void event_button_hair_prev();
  static void event_button_hair_next();

  static void event_button_hair_color_prev();
  static void event_button_hair_color_next();

  static void event_button_skin_color_prev();
  static void event_button_skin_color_next();

  static void event_button_top_prev();
  static void event_button_top_next();

  static void event_button_bottom_prev();
  static void event_button_bottom_next();

  static void event_button_shoes_prev();
  static void event_button_shoes_next();

  static void event_button_weapon_prev();
  static void event_button_weapon_next();

  static void event_button_select();
  static void event_button_new();
  static void event_button_delete();
  static void event_button_back();

  static bool event_button(SDL_Event *event);

  static inline int str_point;
  static inline int dex_point;
  static inline int int_point;
  static inline int luk_point;

  static inline int gender;

public:
  static void reset_character(bool g);
  static bool render();
  static bool event(SDL_Event *event);

  static inline game_character g_character;
};