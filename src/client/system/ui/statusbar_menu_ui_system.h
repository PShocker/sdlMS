#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <optional>
class statusbar_menu_ui_system {
private:
  static void render_backgrnd();
  static void render_button();
  
  static bool event_button(SDL_Event *event);

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