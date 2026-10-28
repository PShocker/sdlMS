#pragma once

#include "src/client/game/game_popup_tip.h"
#include <string>

using namespace fbs;

class PopupTipGameInstance {
private:
  static bool check_type(PopupTipEnums type);

public:
  static void add_tip(PopupTipEnums type, PlayerT player_t);
  static void add_quest_tip(const std::u16string &id);
  
  static inline std::vector<GamePopupTip> data;
};