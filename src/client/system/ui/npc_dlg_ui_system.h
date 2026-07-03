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
  static SDL_FPoint load_wh();

  static void event_button(SDL_Event *event);
  static void event_top();
  static void event_drag_start(SDL_Event *event);
  static void event_drag_end();
  static void event_drag_move(SDL_Event *event);

  static inline std::optional<SDL_FPoint> drag;

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
  static inline uint8_t index;

  static inline std::vector<std::u16string> select;
  static inline std::optional<bool> yesno;
  static inline std::function<void()> cb;
};