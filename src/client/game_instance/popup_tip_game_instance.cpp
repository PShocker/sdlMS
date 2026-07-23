#include "popup_tip_game_instance.h"
#include "src/client/game/game_popup_tip.h"

bool popup_tip_game_instance::check_type(popup_tip_enums type) {
  for (int i = 0; i < data.size(); i++) {
    if (data[i].type == type) {
      return false;
    }
  }
  return true;
}

void popup_tip_game_instance::load_tip(popup_tip_enums type, PlayerT player_t) {
  if (check_type(type)) {
    game_popup_tip tp;
    tp.type = type;
    tp.player_t = player_t;
    data.push_back(tp);
  }
}