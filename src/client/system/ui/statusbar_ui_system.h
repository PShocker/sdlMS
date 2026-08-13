#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include "src/client/game/game_chat.h"
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
  static void render_hm_alert();
  static void render_gauge_text();
  static void render_backgrnd();
  static void render_button();
  static void render_character_stat();
  static void render_chat();
  static void render_back_chat();

  static SDL_FPoint load_wh();

  static void event_chat_send();
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

public:
  static inline std::optional<game_chat_enum> chat_type;
  static inline text_input chat;

  static void reset();
  static bool render();
  static bool event(SDL_Event *event);

  static bool cursor_in();
};