#include "package_game_instance.h"
#include "item_game_instance.h"
#include "src/client/game/game_item.h"
#include <optional>

void package_game_instance::load(const character_save &cs) {
  equips.assign(96, std::nullopt);
  cosumes.assign(96, std::nullopt);
  etc.assign(96, std::nullopt);
  install.assign(96, std::nullopt);
  cash.assign(96, std::nullopt);

  for (auto pkg : cs.package) {
    if (std::holds_alternative<game_equip>(pkg.val)) {
      game_equip &equip = std::get<game_equip>(pkg.val);
      equips[pkg.index] = equip;
    } else {
      game_item &item = std::get<game_item>(pkg.val);
      auto item_type = item_game_instance::load_item_type(item.id);
      if (item_type == u"Consume") {
        cosumes[pkg.index] = item;
      } else if (item_type == u"Etc") {
        etc[pkg.index] = item;
      } else if (item_type == u"Install") {
        install[pkg.index] = item;
      } else {
        cash[pkg.index] = item;
      }
    }
  }
  meso = cs.meso;
}