#include "item_buff_game_instance.h"
#include "src/client/game/game_item_buff.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/window/window.h"
#include "wz/Property.h"

void item_buff_game_instance::use(const std::u16string &id) {
  auto info = item_game_instance::load_item_info(id, 0);
  info = info->find(u"../spec");
  auto time = static_cast<wz::Property<int> *>(info->get_child(u"time"))->get();
  game_item_buff gib;
  gib.destroy = window::dt_now + time;
  gib.id = id;
  end(id);

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
      break;
    }
  }
}