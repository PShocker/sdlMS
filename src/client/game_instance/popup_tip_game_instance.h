#pragma once

#include "src/client/game/game_popup_tip.h"

using namespace fbs;

class popup_tip_game_instance {
private:
  static bool check_type(popup_tip_enums type);

public:
  static void load_tip(popup_tip_enums type, PlayerT player_t);
  static inline std::vector<game_popup_tip> data;
};