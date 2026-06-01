#pragma once

#include "SDL3/SDL_events.h"
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

  static void event_chat();
  static void event_chat_edit(const char *text);
  static void event_chat_input(const char *text);
  static void event_chat_del();

  enum chat_type {
    all,
  };
  static std::u16string load_chat_type_str();

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

  static inline bool quickSlot = true;

  static inline std::optional<std::u16string> chat;
  static inline std::optional<std::u16string> chat2;
  static inline std::optional<chat_type> chat_type;

public:
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
};