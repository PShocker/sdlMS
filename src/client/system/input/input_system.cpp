#include "input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/keybinding_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include <flat_map>
#include <string>

void input_system::handle_ui_input(game_input &g_input) {
  static const std::flat_map<std::string, void (*)()> fns = {
      {"worldMap", worldmap_ui_system::toggle},
      {"package", package_ui_system::toggle},
      {"characterStat", character_stat_ui_system::toggle},
      {"characterInfo", character_info_ui_system::toggle},
      {"keyBind", keybinding_ui_system::toggle},
      {"skill", skill_ui_system::toggle},
      {"equip", equip_ui_system::toggle},
      {"craft", craft_ui_system::toggle},
      {"quest", quest_ui_system::toggle},
  };
  fns.at(g_input.val)();
}

void input_system::handle_item_input(game_input &g_input) {
  static const std::flat_map<std::string, void (*)()> fns = {
      {"worldMap", worldmap_ui_system::toggle},
  };
  fns.at(g_input.val)();
}

void input_system::handle_action_input(game_input &g_input, bool press) {
  if (press) {
    character_logic_system::character_action_input.insert(g_input.val2);
  } else {
    character_logic_system::character_action_input.erase(g_input.val2);
  }
}

void input_system::handle_skill_input(game_input &g_input, bool press) {
  if (press) {
    character_logic_system::character_skill_input.insert(g_input.val2);
  } else {
    character_logic_system::character_skill_input.erase(g_input.val2);
  }
}