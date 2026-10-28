#include "src/client/system/input/input_system.h"

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

namespace sdlms {

void InputSystem::HandleUiInput(GameInput& g_input) {
  static const std::flat_map<std::string, void (*)()> fns = {
      {"worldMap", WorldmapUiSystem::Toggle},
      {"package", PackageUiSystem::Toggle},
      {"characterStat", CharacterStatUiSystem::Toggle},
      {"characterInfo", CharacterInfoUiSystem::Toggle},
      {"keyBind", KeybindingUiSystem::Toggle},
      {"skill", SkillUiSystem::Toggle},
      {"equip", EquipUiSystem::Toggle},
      {"craft", CraftUiSystem::Toggle},
      {"quest", QuestUiSystem::Toggle},
      {"option", OptionUiSystem::Toggle},
      {"menu", StatusbarMenuUiSystem::Toggle},
  };
  if (fns.contains(g_input.val)) {
    fns.at(g_input.val)();
  }
}

void InputSystem::HandleItemInput(GameInput& g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_item_input_.insert(g_input);
  } else {
    CharacterLogicSystem::character_item_input_.erase(g_input);
  }
}

void InputSystem::HandleActionInput(GameInput& g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_action_input_.insert(g_input.val);
  } else {
    CharacterLogicSystem::character_action_input_.erase(g_input.val);
  }
}

void InputSystem::HandleSkillInput(GameInput& g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_skill_input_.insert(g_input.val);
  } else {
    CharacterLogicSystem::character_skill_input_.erase(g_input.val);
  }
}

void InputSystem::HandleFaceInput(GameInput& g_input, bool press) {
  if (press) {
    CharacterLogicSystem::character_face_input_.insert(g_input.val);
  } else {
    CharacterLogicSystem::character_face_input_.erase(g_input.val);
  }
}

void InputSystem::Reset() {
  CharacterLogicSystem::character_item_input_.clear();
  CharacterLogicSystem::character_action_input_.clear();
  CharacterLogicSystem::character_skill_input_.clear();
  CharacterLogicSystem::character_face_input_.clear();
}

}  // namespace sdlms
