#include "package_game_instance.h"
#include "item_game_instance.h"
#include "src/client/game/game_item.h"
#include <algorithm>
#include <memory>
#include <optional>

void package_game_instance::load(const character_save &cs) {
  for (auto &d : data) {
    d.clear(); // 先清空
  }
  data[0].assign(96, std::polymorphic<game_item>(game_equip_item{}));
  data[1].assign(96, std::polymorphic<game_item>(game_consume_item{}));
  data[2].assign(96, std::polymorphic<game_item>(game_etc_item{}));
  data[3].assign(96, std::polymorphic<game_item>(game_install_item{}));
  data[4].assign(96, std::polymorphic<game_item>(game_cash_item{}));
  data[5].assign(96, std::polymorphic<game_item>(game_deco_item{}));

  // meso = 0;
  data[0][0]->id=u"01472012";

  for (auto &pkg : cs.package) {
    auto type = (int)pkg.val->type;
    data[type][pkg.index] = pkg.val;
  }
  // meso = cs.meso;
}