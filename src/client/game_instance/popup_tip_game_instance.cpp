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
  AudioGameInstance::LoadAudio(u"UI.img/Invite", 0);
}

void PopupTipGameInstance::AddQuestTip(const std::u16string& id) {
  GamePopupTip tp;
  tp.type = PopupTipEnums::kQuest;
  tp.data = id;
  tp.destroy = Window::dt_now_ + 5 * 1000;
  data_.emplace_back(tp);
  // 任务条件满足 / 新的任务 pop_tip
  AudioGameInstance::LoadAudio(u"UI.img/Invite", 0);
}

}  // namespace sdlms
