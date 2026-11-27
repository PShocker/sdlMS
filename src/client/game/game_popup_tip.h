#pragma once

#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"

#include <any>
#include <cstdint>

namespace sdlms {

enum class PopupTipEnums {
  kTrade,
  kParty,
  kQuest,
  // 五子棋房间内对方的请求（悔棋/求和，FadeYesNo 确认框）。
  kMinigame,
  // MapleChat 聊天邀请（095 CUIFadeYesNo CreateMSMInvite）。
  kMapleChat,
  // 好友上线通知（纯提示，无按钮，自动消失；data = 好友名字 u16string）。
  kFriendOnline,
};

// 五子棋请求提示：kind 对应 ClientCharacterMinigame 的 6=悔棋 / 8=求和。
struct GameOmokRequestTip {
  uint8_t kind = 0;
  fbs::PlayerT player;
};

// MapleChat 聊天邀请提示：sn = 邀请人所在房间 SN。
struct GameMapleChatInviteTip {
  uint64_t sn = 0;
  fbs::PlayerT player;
};

struct GamePopupTip {
  PopupTipEnums type;
  int32_t alpha = 0;
  uint64_t destroy = Window::dt_now_ + 120 * 1000;
  std::any data;
};

}  // namespace sdlms
