#include "input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/ui/character_info_ui_system.h"
#include "src/client/system/ui/character_stat_ui_system.h"
#include "src/client/system/ui/craft_ui_system.h"
#include "src/client/system/ui/equip_ui_system.h"
#include "src/client/system/ui/keybinding_ui_system.h"
#include "src/client/system/ui/option_ui_system.h"
#include "src/client/system/ui/package_ui_system.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/client/system/ui/skill_ui_system.h"
#include "src/client/system/ui/statusbar_menu_ui_system.h"
#include "src/client/system/ui/worldmap_ui_system.h"
#include <flat_map>
#include <string>

void InputSystem::handle_ui_input(GameInput &g_input) {
  static const std::flat_map<std::string, void (*)()> fns = {
      {"worldMap", WorldmapUiSystem::toggle},
      {"package", PackageUiSystem::toggle},
      {"characterStat", CharacterStatUiSystem::toggle},
      {"characterInfo", CharacterInfoUiSystem::toggle},
      {"keyBind", KeybindingUiSystem::toggle},
      {"skill", SkillUiSystem::toggle},
      {"equip", EquipUiSystem::toggle},
      {"craft", CraftUiSystem::toggle},
      {"quest", QuestUiSystem::toggle},
      {"option", OptionUiSystem::toggle},
      {"menu", StatusbarMenuUiSystem::toggle},
  };
  if (fns.contains(g_input.val)) {
    fns.at(g_input.val)();
  }
}

void InputSystem::handle_item_input(GameInput &g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_item_input.insert(g_input);
  } else {
    CharacterLogicSystem::character_item_input.erase(g_input);
  }
}

void InputSystem::handle_action_input(GameInput &g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_action_input.insert(g_input.val);
  } else {
    CharacterLogicSystem::character_action_input.erase(g_input.val);
  }
}

void InputSystem::handle_skill_input(GameInput &g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_skill_input.insert(g_input.val);
  } else {
    CharacterLogicSystem::character_skill_input.erase(g_input.val);
  }
}

void InputSystem::handle_face_input(GameInput &g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_face_input.insert(g_input.val);
  } else {
    CharacterLogicSystem::character_face_input.erase(g_input.val);
  }
}

void InputSystem::reset() {
  CharacterLogicSystem::character_item_input.clear();
  CharacterLogicSystem::character_action_input.clear();
  CharacterLogicSystem::character_skill_input.clear();
  CharacterLogicSystem::character_face_input.clear();
}