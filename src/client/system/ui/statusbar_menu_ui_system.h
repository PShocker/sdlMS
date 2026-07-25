#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
class statusbar_menu_ui_system {
private:
  static SDL_FRect load_r();
  static void render_backgrnd();
  static void render_button();
  static void render_title();

  static void event_button_cc();
  static void event_button_option();
  static void event_button_quit();

  static void event_button_item();
  static void event_button_equip();
  static void event_button_stat();
  static void event_button_skill();
  static void event_button_party();
  static void event_button_quest();
  static void event_button_craft();


  static bool event_button(SDL_Event *event);

  static inline int alpha;

public:
  enum class menu_enums {
    menu,
    shortcut,
  };
  static inline std::optional<menu_enums> menu_type;

  static bool cursor_in();

  static void toggle();

  static void open();
  static void close();
  static bool render();
  static bool event(SDL_Event *event);
};