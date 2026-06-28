#pragma once

#include "SDL3/SDL_events.h"
#include "text_input_ui_system.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>

class statusbar_ui_system {
private:
  static void render_gauge_text();
  static void render_backgrnd();
  static void render_button();
  static void render_quickSlot();
  static void render_character_stat();
  static void render_chat();
  static SDL_FPoint load_wh();

  static void event_chat_send();
  static bool event_click_quickslot(SDL_Event *event);

  enum chat_type {
    all,
  };
  static std::u16string load_chat_type();

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

  enum class quick_slot {
    hide,
    two,
    three,
  };
  static inline quick_slot quickSlot = quick_slot::two;

  static inline std::optional<chat_type> chat_type;
  static inline text_input chat;

public:
  static void reset();
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
};