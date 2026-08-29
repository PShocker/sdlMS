#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_rect.h"
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
class npc_dlg_ui_system {
private:
  static void render_backgrnd();
  static void render_npc();
  static void render_button();
  static void render_text();
  static void render_list();

  static SDL_FPoint load_wh();

  static void event_quest_list();

  static void event_button_close();
  static void event_button_ok();

  static void event_button_prev();
  static void event_button_next();

  static void event_button_quest_yes();
  static void event_button_quest_no();

  static bool event_button(SDL_Event *event);

public:
  static bool render();

  static void open();
  static void close();

  static bool event(SDL_Event *event);

  static bool cursor_in();

  static void toggle();

  static inline SDL_FPoint pos;

  static inline std::u16string npc_id;
  static inline std::u16string text;
  static inline std::u16string quest_id;
  static inline std::u16string quest_index;
  static inline uint8_t index;
  static inline uint8_t max_index;

  enum class npc_dlg_enum {
    choose,
    quest,
    talk,
    select,
  };
  static inline npc_dlg_enum type;

  static inline uint64_t time;
  static inline std::function<void()> cb;
};