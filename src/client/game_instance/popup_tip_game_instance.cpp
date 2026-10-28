#include "popup_tip_game_instance.h"
#include "src/client/game/game_popup_tip.h"

bool PopupTipGameInstance::CheckType(PopupTipEnums type) {
  for (int i = 0; i < data.size(); i++) {
    if (data[i].type == type) {
      return false;
    }
  }
  return true;
}

void PopupTipGameInstance::AddTip(PopupTipEnums type, PlayerT player_t) {
  if (!CheckType(type)) {
    return;
  }
  GamePopupTip tp;
  tp.type = type;
  tp.data = player_t;
  data.emplace_back(tp);
}

void PopupTipGameInstance::AddQuestTip(const std::u16string &id) {
  GamePopupTip tp;
  tp.type = PopupTipEnums::kQuest;
  tp.data = id;
  tp.destroy = Window::dt_now + 5 * 1000;
  data.emplace_back(tp);
}