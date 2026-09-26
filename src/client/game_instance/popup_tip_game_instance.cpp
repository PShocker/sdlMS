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

void popup_tip_game_instance::add_tip(popup_tip_enums type, PlayerT player_t) {
  if (!check_type(type)) {
    return;
  }
  game_popup_tip tp;
  tp.type = type;
  tp.data = player_t;
  data.emplace_back(tp);
}

void popup_tip_game_instance::add_quest_tip(const std::u16string &id) {
  game_popup_tip tp;
  tp.type = popup_tip_enums::quest;
  tp.data = id;
  tp.destroy = window::dt_now + 5 * 1000;
  data.emplace_back(tp);
}