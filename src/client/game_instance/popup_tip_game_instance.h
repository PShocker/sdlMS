#pragma once

#include "src/client/game/game_popup_tip.h"
#include <string>

using namespace fbs;

class popup_tip_game_instance {
private:
  static bool check_type(popup_tip_enums type);

public:
  static void add_tip(popup_tip_enums type, PlayerT player_t);
  static void add_quest_tip(const std::u16string &id);
  
  static inline std::vector<game_popup_tip> data;
};