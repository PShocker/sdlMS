#include "popup_tip_game_instance.h"
#include "src/client/game/game_popup_tip.h"

bool PopupTipGameInstance::check_type(PopupTipEnums type) {
  for (int i = 0; i < data.size(); i++) {
    if (data[i].type == type) {
      return false;
    }
  }
  return true;
}

void PopupTipGameInstance::add_tip(PopupTipEnums type, PlayerT player_t) {
  if (!check_type(type)) {
    return;
  }
  GamePopupTip tp;
  tp.type = type;
  tp.data = player_t;
  data.emplace_back(tp);
}

void PopupTipGameInstance::add_quest_tip(const std::u16string &id) {
  GamePopupTip tp;
  tp.type = PopupTipEnums::quest;
  tp.data = id;
  tp.destroy = Window::dt_now + 5 * 1000;
  data.emplace_back(tp);
}