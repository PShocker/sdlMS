#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_popup_tip.h"
#include "src/common/flatbuffers/server.h"
#include "text_input_ui_system.h"
#include <cstdint>
#include <flat_map>
#include <optional>
#include <string>
#include <vector>

class statusbar_ui_system {
private:
  static void render_gauge_text();
  static void render_backgrnd();
  static void render_button();
  static void render_quickSlot();
  static void render_character_stat();
  static void render_chat();
  static void render_chat_info();
  static void render_chat_infos();
  static void render_chat_vscr();
  static void render_popup_tips();

  static void render_submenu_backgrnd();
  static void render_submenu_button();
  static void render_submenu();

  static SDL_FPoint load_wh();

  static void event_click_chat_vscr();

  static void event_chat_send();
  static bool event_click_quickslot(SDL_Event *event);

  static std::u16string load_chat_type();

  static bool event_menu_button(SDL_Event *event);
  static void event_menu_button_option();

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

  static inline int chat_index;

  enum class menu_enums {
    menu,
    shortcut,
  };
  static inline std::optional<menu_enums> menu_type;

public:
  enum class quick_slot {
    hide,
    two,
    three,
  };
  static inline quick_slot quickSlot = quick_slot::two;

  enum chat_enum {
    all,
  };
  struct chats {
    chat_enum type;
    std::u16string owner;
    std::u16string text;
  };
  static inline std::vector<chats> chats_info;

  static inline std::vector<game_popup_tip> tips;
  static inline std::optional<chat_enum> chat_type;
  static inline text_input chat;

  static void reset();
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
};