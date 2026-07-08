#include "package_game_instance.h"
#include "item_game_instance.h"
#include "src/client/game/game_item.h"
#include <optional>

void package_game_instance::load(const character_save &cs) {
  for (auto &d : data) {
    d.assign(96, std::nullopt);
  }
  for (auto pkg : cs.package) {
    auto type = (int)pkg.val.type;
    data[type][pkg.index] = pkg.val;
  }
  meso = cs.meso;
}