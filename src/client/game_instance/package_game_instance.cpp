#include "package_game_instance.h"
#include "src/client/game/game_item.h"

void package_game_instance::load() {
  game_equip e{
      .id = u"01382004",
  };
  equips[0] = e;
  game_item i{
      .id = u"04080000",
      .num = 1,
  };
  cosumes[0] = i;
}