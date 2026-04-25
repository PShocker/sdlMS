#include "cursor_logic_system.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include "src/client/window/window.h"
#include <cstdlib>

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

void cursor_logic_system::run_cursor_ui() {
  for (auto &fn : system::render_systems) {
    if (fn == minimap_ui_system::render) {
      if (minimap_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == statusbar_ui_system::render) {
      if (statusbar_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == worldmap_ui_system::render) {
      if (worldmap_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == equip_ui_system::render) {
      if (equip_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == package_ui_system::render) {
      if (package_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    }
  }
}

bool cursor_logic_system::run() {
  for (auto &fn : {run_mousedown, run_default}) {
    if (fn()) {
      break;
    }
  }
  run_cursor_ui();
  return true;
}