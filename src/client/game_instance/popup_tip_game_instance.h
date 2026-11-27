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
  // 五子棋房间内的悔棋/求和请求（FadeYesNo 确认框）。
  static void AddOmokTip(uint8_t kind, fbs::PlayerT player_t);
  // MapleChat 聊天邀请（095 CUIFadeYesNo CreateMSMInvite）。
  static void AddMapleChatTip(uint64_t sn, fbs::PlayerT player_t);
  // 好友上线通知（纯提示，无按钮，自动消失）。
  static void AddFriendOnlineTip(const std::u16string& name);

  // 登录读档前清理旧提示，并暂时禁止任务完成提示。
  static void Load();
  static void EndLoad();

  // 登录读档恢复任务时禁止弹出已完成任务的提示。
  static inline bool suppress_quest_tip_ = false;

  static inline std::vector<GamePopupTip> data_;
};

}  // namespace sdlms
