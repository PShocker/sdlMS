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
  game_consume_item gci;
  gci.id = u"02070008";
  gci.num = 1000;

  data[(int)item_enum::equip].assign(
      96, std::polymorphic<game_item>(game_equip_item{}));
  data[(int)item_enum::consume].assign(96, std::polymorphic<game_item>(gci));
  data[(int)item_enum::install].assign(
      96, std::polymorphic<game_item>(game_install_item{}));
  data[(int)item_enum::etc].assign(
      96, std::polymorphic<game_item>(game_etc_item{}));
  data[(int)item_enum::cash].assign(
      96, std::polymorphic<game_item>(game_cash_item{}));
  data[(int)item_enum::deco].assign(
      96, std::polymorphic<game_item>(game_deco_item{}));

  gci.id = u"02000000";
  gci.num = 1000;
  data[(int)item_enum::consume][0] = std::polymorphic<game_item>(gci);

  // meso = 0;
  data[0][0]->id = u"01472012";

  for (auto &pkg : cs.package) {
    auto type = (int)pkg.val->type;
    data[type][pkg.index] = pkg.val;
  }
  // meso = cs.meso;
}