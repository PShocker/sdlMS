#include "package_game_instance.h"
#include "item_game_instance.h"
#include "src/client/game/game_item.h"
#include <algorithm>
#include <memory>
#include <optional>

void package_game_instance::load(const character_save &cs) {
  for (auto &d : data) {
    d.clear();    // 先清空
    d.resize(96); // 创建96个空的 unique_ptr
  }
  for (auto &pkg : cs.package) {
    auto type = (int)pkg.val->type;
    switch ((item_enum)type) {
    case item_enum::equip: {
      auto *eqp = static_cast<const game_equip_item *>(pkg.val.get());
      data[type][pkg.index] = std::make_unique<game_equip_item>(*eqp);
      break;
    }
    case item_enum::consume: {
      auto *con = static_cast<const game_consume_item *>(pkg.val.get());
      data[type][pkg.index] = std::make_unique<game_consume_item>(*con);
      break;
    }
    case item_enum::etc: {
      auto *etc = static_cast<const game_etc_item *>(pkg.val.get());
      data[type][pkg.index] = std::make_unique<game_etc_item>(*etc);
      break;
    }
    case item_enum::install: {
      auto *ins = static_cast<const game_install_item *>(pkg.val.get());
      data[type][pkg.index] = std::make_unique<game_install_item>(*ins);
      break;
    }
    case item_enum::cash: {
      auto *cas = static_cast<const game_cash_item *>(pkg.val.get());
      data[type][pkg.index] = std::make_unique<game_cash_item>(*cas);
      break;
    }
    }
  }
  meso = cs.meso;
}