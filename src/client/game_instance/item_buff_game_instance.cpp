#include "item_buff_game_instance.h"
#include "src/client/game/game_item_buff.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "wz/Property.h"

void item_buff_game_instance::use(const std::u16string &id) {
  end(id);
  auto info = item_game_instance::load_item_info(id, 0);
  info = info->find(u"../spec");
  auto time = static_cast<wz::Property<int> *>(info->get_child(u"time"))->get();
  if (info->get_child(u"pad")) {
    auto pad = static_cast<wz::Property<int> *>(info->get_child(u"pad"))->get();
    character_stat_game_instance::itm_attack += pad;
  }
  game_item_buff gib;
  gib.destroy = window::dt_now + time;
  gib.id = id;
  data.emplace_back(gib);
}

void item_buff_game_instance::end(const std::u16string &id) {
  for (auto &gib : data) {
    if (gib.id == id) {
      auto info = item_game_instance::load_item_info(id, 0);
      info = info->find(u"../spec");
      if (info->get_child(u"pad")) {
        auto pad =
            static_cast<wz::Property<int> *>(info->get_child(u"pad"))->get();
        character_stat_game_instance::itm_attack -= pad;
      }
      if (info->get_child(u"mad")) {
        auto mad =
            static_cast<wz::Property<int> *>(info->get_child(u"mad"))->get();
        character_stat_game_instance::itm_magic -= mad;
      }
      if (info->get_child(u"acc")) {
        auto acc =
            static_cast<wz::Property<int> *>(info->get_child(u"acc"))->get();
        character_stat_game_instance::itm_accuracy -= acc;
      }
      if (info->get_child(u"eva")) {
        auto eva =
            static_cast<wz::Property<int> *>(info->get_child(u"eva"))->get();
        character_stat_game_instance::itm_avoid -= eva;
      }
      if (info->get_child(u"speed")) {
        auto speed =
            static_cast<wz::Property<int> *>(info->get_child(u"speed"))->get();
        character_logic_system::self_hspeed_max -= 5;
        character_logic_system::self_hspeed_min += 5;
      }
      if (info->get_child(u"crt")) {
        auto crt =
            static_cast<wz::Property<int> *>(info->get_child(u"crt"))->get();
        character_stat_game_instance::crit_damage -= crt;
      }
      break;
    }
  }
}