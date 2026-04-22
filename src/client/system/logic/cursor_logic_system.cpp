#include "cursor_logic_system.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/window/window.h"

bool cursor_logic_system::run_mousedown() {
  if (window::mouse_state & SDL_BUTTON_LMASK) {
    cursor_game_instance::cursor_type = u"12";
    cursor_game_instance::cursor_index = 0;
    cursor_game_instance::cursor_time = 0;
    return true;
  }
  return false;
}

bool cursor_logic_system::run_default() {
  cursor_game_instance::cursor_type = u"0";
  cursor_game_instance::cursor_index = 0;
  cursor_game_instance::cursor_time = 0;
  return true;
}

bool cursor_logic_system::run() {
  for (auto &fn : {run_mousedown, run_default}) {
    if (fn()) {
      break;
    }
  }
  return true;
}