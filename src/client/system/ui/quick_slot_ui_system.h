#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "SDL3/SDL_scancode.h"
#include "src/client/game/game_input.h"
#include <cstdint>
class QuickSlotUiSystem {
private:
  static void render_quickSlot();
  static void render_button();

  static bool event_button(SDL_Event *event);
  static bool event_click_quickslot(SDL_Event *event);

  static SDL_FPoint load_wh();

public:
  enum class QuickSlot {
    hide,
    two,
    three,
  };
  static inline QuickSlot quickSlot = QuickSlot::two;

  static void open();
  static void close();

  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();

  static void render_input_icon(const GameInput &i, int x, int y,
                                uint8_t alpha = 255);
};