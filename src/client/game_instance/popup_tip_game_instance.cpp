#include "src/client/game_instance/popup_tip_game_instance.h"

#include "src/client/game/game_popup_tip.h"
#include "src/client/game_instance/audio_game_instance.h"

namespace sdlms {

bool PopupTipGameInstance::CheckType(PopupTipEnums type) {
  for (int i = 0; i < data_.size(); i++) {
    if (data_[i].type == type) {
      return false;
    }
  }
  return true;
}

void PopupTipGameInstance::AddTip(PopupTipEnums type, fbs::PlayerT player_t) {
  if (!CheckType(type)) {
    return;
  }
  GamePopupTip tp;
  tp.type = type;
  tp.data = player_t;
  data_.emplace_back(tp);
  AudioGameInstance::PlayNow(u"UI.img/Invite");
}

void PopupTipGameInstance::Load() {
  data_.clear();
  suppress_quest_tip_ = true;
}

void PopupTipGameInstance::EndLoad() {
  suppress_quest_tip_ = false;
}

void PopupTipGameInstance::AddQuestTip(const std::u16string& id) {
  if (suppress_quest_tip_) {
    return;
  }
  GamePopupTip tp;
  tp.type = PopupTipEnums::kQuest;
  tp.data = id;
  tp.destroy = Window::dt_now_ + 5 * 1000;
  data_.emplace_back(tp);
  // 任务条件满足 / 新的任务 pop_tip
  AudioGameInstance::PlayNow(u"UI.img/Invite");
}

void PopupTipGameInstance::AddOmokTip(uint8_t kind, fbs::PlayerT player_t) {
  if (!CheckType(PopupTipEnums::kMinigame)) {
    return;
  }
  GamePopupTip tp;
  tp.type = PopupTipEnums::kMinigame;
  GameOmokRequestTip req;
  req.kind = kind;
  req.player = std::move(player_t);
  tp.data = std::move(req);
  data_.emplace_back(tp);
  AudioGameInstance::PlayNow(u"UI.img/Invite");
}

void PopupTipGameInstance::AddMapleChatTip(uint64_t sn, fbs::PlayerT player_t) {
  if (!CheckType(PopupTipEnums::kMapleChat)) {
    return;
  }
  GamePopupTip tp;
  tp.type = PopupTipEnums::kMapleChat;
  GameMapleChatInviteTip inv;
  inv.sn = sn;
  inv.player = std::move(player_t);
  tp.data = std::move(inv);
  data_.emplace_back(tp);
  AudioGameInstance::PlayNow(u"UI.img/Invite");
}

void PopupTipGameInstance::AddFriendOnlineTip(const std::u16string& name) {
  // 纯通知：不占用确认框，和任务提示一样自动消失。
  if (!CheckType(PopupTipEnums::kFriendOnline)) {
    return;
  }
  GamePopupTip tp;
  tp.type = PopupTipEnums::kFriendOnline;
  tp.data = name;
  tp.destroy = Window::dt_now_ + 6 * 1000;
  data_.emplace_back(tp);
  AudioGameInstance::PlayNow(u"UI.img/Invite");
}

}  // namespace sdlms
