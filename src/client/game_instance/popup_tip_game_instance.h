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

  static inline std::vector<GamePopupTip> data_;
};

}  // namespace sdlms
