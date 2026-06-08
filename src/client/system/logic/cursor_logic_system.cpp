#include "cursor_logic_system.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/minimap_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/revive_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system/ui/statusbar_ui_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/wz/wz_resource.h"
#include "wz/Property.h"
#include "wz/Wz.h"
#include <cstdlib>
#include <string>

bool cursor_logic_system::run_package_motion() {
  auto index = package_ui_system::load_mouse_index();
  if (!index.has_value()) {
    return false;
  }
  auto active_tab = package_ui_system::active_tab;
  if (active_tab == 0) {
    if (!package_game_instance::equips[index.value()].has_value()) {
      return false;
    }
  } else {
    std::vector<std::optional<game_item>> *r;
    switch (active_tab) {
    case 1: {
      r = &package_game_instance::cosumes;
      break;
    }
    case 2: {
      r = &package_game_instance::etc;
      break;
    }
    case 3: {
      r = &package_game_instance::install;
      break;
    }
    case 4: {
      r = &package_game_instance::cash;
      break;
    }
    default: {
      break;
    }
    }
    if (!r->at(index.value()).has_value()) {
      return false;
    }
  }
  return true;
}

void cursor_logic_system::run_cursor_action(const std::u16string &action) {
  if (cursor_game_instance::cursor_type != action) {
    cursor_game_instance::cursor_type = action;
    cursor_game_instance::cursor_index = 0;
    cursor_game_instance::cursor_time = 0;
  }
}

bool cursor_logic_system::run_default() {
  if (cursor_game_instance::cursor_hand.has_value()) {
    run_cursor_action(u"11");
    return true;
  }
  if (cursor_game_instance::cursor_ui == package_ui_system::render) {
    if (run_package_motion()) {
      run_cursor_action(u"5");
      return true;
    }
  }
  if (window::mouse_state & SDL_BUTTON_LMASK) {
    run_cursor_action(u"12");
  } else {
    run_cursor_action(u"0");
  }

  return true;
}

void cursor_logic_system::run_cursor_ui() {
  cursor_game_instance::cursor_ui = nullptr;
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
    } else if (fn == character_stat_ui_system::render) {
      if (character_stat_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == character_info_ui_system::render) {
      if (character_info_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == skill_ui_system::render) {
      if (skill_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == craft_ui_system::render) {
      if (craft_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    } else if (fn == revive_ui_system::render) {
      if (revive_ui_system::cursor_in()) {
        cursor_game_instance::cursor_ui = fn;
      }
    }
  }
}

bool cursor_logic_system::run_animate() {
  static auto cursor_node = wz_resource::ui->find(u"Cursor.img");
  auto type_node = cursor_node->get_child(cursor_game_instance::cursor_type);
  auto child_count = type_node->children_count();
  auto index = std::to_string(cursor_game_instance::cursor_index);
  type_node = type_node->get_child(index);
  if (type_node->type == wz::Type::UOL) {
    type_node = static_cast<wz::Property<wz::WzUOL> *>(type_node)->get_uol();
  }
  int delay = 0;
  if (type_node->get_child(u"delay")) {
    delay =
        static_cast<wz::Property<int> *>(type_node->get_child(u"delay"))->get();
  }
  cursor_game_instance::cursor_time += window::delta_time;
  if (cursor_game_instance::cursor_time >= delay) {
    cursor_game_instance::cursor_time = 0;
    cursor_game_instance::cursor_index += 1;
    cursor_game_instance::cursor_index =
        cursor_game_instance::cursor_index % child_count;
  }
  return true;
}

bool cursor_logic_system::run() {
  run_animate();
  run_cursor_ui();
  run_default();
  return true;
}