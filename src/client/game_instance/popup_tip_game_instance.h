#pragma once

#include "src/client/game/game_popup_tip.h"

#include <string>

namespace sdlms {

class PopupTipGameInstance {
 private:
  static bool CheckType(PopupTipEnums type);

 public:
  static void AddTip(PopupTipEnums type, fbs::PlayerT player_t);
  static void AddQuestTip(const std::u16string& id);

  // 登录读档前清理旧提示，并暂时禁止任务完成提示。
  static void Load();
  static void EndLoad();

  // 登录读档恢复任务时禁止弹出已完成任务的提示。
  static inline bool suppress_quest_tip_ = false;

  static inline std::vector<GamePopupTip> data_;
};

}  // namespace sdlms
